#include "magneto.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kTimeoutMs=500;
static const int8_t kInput=10;
bool end=false;

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);
  client.Write("agent", magneto::Buf(Slice(RCAST<const char*>(&kInput), 1), NULL), kTimeoutMs);
  client.FreeTalks();
  end=true;
}

void AgentService(const magneto::ProtocolRead& protocol_read, void* args) {
  magneto::Magneto& agent = *RCAST<magneto::Magneto*>(args);
  if (magneto::Protocol::kRapid == protocol_read.GetCategory()) {
    const magneto::ProtocolReadRapid& protocol_read_rapid = SCAST<const magneto::ProtocolReadRapid&>(protocol_read);

    magneto::Buf buf(Slice(protocol_read_rapid.Data(), 1), NULL);
    magneto::Bufs bufs;
    bufs.push_back(&buf);
    bufs.push_back(&buf);
    magneto::Errors errors;
    agent.Write("consumers", bufs, kTimeoutMs, errors);
  } else if (magneto::Protocol::kPing == protocol_read.GetCategory()) {
    agent.WriteBack(magneto::Buf(Slice("p", 1), NULL), kTimeoutMs);
  }
}

void ConsumerService(const magneto::ProtocolRead& protocol_read, void* /*args*/) {
  const magneto::ProtocolReadRapid& protocol_read_rapid = SCAST<const magneto::ProtocolReadRapid&>(protocol_read);
  assert( kInput == *(protocol_read_rapid.Data()) );
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  static const size_t kNumServers=4;
  magneto::Magneto* servers = new magneto::Magneto [kNumServers];
  magneto::Magneto* client = &(servers[0]);
  magneto::Magneto* agent = &(servers[1]);
  magneto::Magneto* consumer0 = &(servers[2]);
  magneto::Magneto* consumer1 = &(servers[3]);
  magneto::RoutineItems client_handle;

  int ret = agent->Init("conf/agent/", &AgentService, NULL, agent, end);
  assert(true == ret);

  ret = consumer0->Init("conf/consumer0/", &ConsumerService, NULL, consumer0, end);
  assert(true == ret);

  ret = consumer1->Init("conf/consumer1/", &ConsumerService, NULL, consumer1, end);
  assert(true == ret);

  client_handle.push_back(std::make_pair(ClientHandler, 1));
  ret = client->Init("conf/client/", NULL, &client_handle, client, end);
  assert(true == ret);

  ret = agent->Start() && consumer0->Start() && consumer1->Start() && client->Start();
  assert(true == ret);

  delete [] servers;
  return 0;
}
