#include "magneto.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kTimeoutMs=500;
bool end=false;

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  int8_t kInput=5;
  magneto::ProtocolRead* protocol_read;

  int ret = client.SimpleTalk("agent", magneto::Buf(Slice(RCAST<const char*>(&kInput), 1), NULL), kTimeoutMs, protocol_read);
  const magneto::ProtocolReadRapid& protocol_read_rapid = 
    SCAST<const magneto::ProtocolReadRapid&>(*protocol_read);
  assert(magneto::ErrorNo::kOk == ret && kInput*2 == *(protocol_read_rapid.Data()));
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
    magneto::Responses responses;
    int8_t resp=0;
    int ret = agent.ParaTalks("plus_minus", bufs, kTimeoutMs, responses);
    if (0==ret) {
      for (size_t i=0; i < responses.size(); ++i) {
        resp += ( 0 == responses[i].first ? *(responses[i].second->Data()) : 0);
      }
    } else {
      resp=0;
    }

    buf = magneto::Buf(Slice(RCAST<const char*>(&resp), 1), NULL);
    agent.WriteBack(buf, kTimeoutMs);
  } else if (magneto::Protocol::kPing == protocol_read.GetCategory()) {
    agent.WriteBack(magneto::Buf(Slice("p", 1), NULL), kTimeoutMs);
  }
}

inline void CalcService(const magneto::ProtocolRead& protocol_read, void* args) {
  const magneto::ProtocolReadRapid& protocol_read_rapid = SCAST<const magneto::ProtocolReadRapid&>(protocol_read);
  char resp;
  if (!strcmp("plus", protocol_read.GetReqInfo().listen_addr->name.c_str())) {
    resp = *(protocol_read_rapid.Data()) + 1;
  } else {
    resp = *(protocol_read_rapid.Data()) - 1;
  }
  magneto::Buf buf(Slice(&resp, 1), NULL);
  RCAST<magneto::Magneto*>(args)->WriteBack(buf, kTimeoutMs);
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  static const size_t kNumServers=6;

  magneto::Magneto* servers = new magneto::Magneto [kNumServers];
  magneto::Magneto* client = &(servers[0]);
  magneto::Magneto* agent = &(servers[1]);
  magneto::Magneto* plus_master = &(servers[2]);
  magneto::Magneto* plus_slave = &(servers[3]);
  magneto::Magneto* minus_master = &(servers[4]);
  magneto::Magneto* minus_slave = &(servers[5]);
  magneto::RoutineItems client_handle;

  int ret = agent->Init("conf/agent/", &AgentService, NULL, agent, end);
  assert(true == ret);

  ret = plus_master->Init("conf/plus_master/", &CalcService, NULL, plus_master, end);
  assert(true == ret);

  ret = plus_slave->Init("conf/plus_slave/", &CalcService, NULL, plus_slave, end);
  assert(true == ret);

  ret = minus_master->Init("conf/minus_master/", &CalcService, NULL, minus_master, end);
  assert(true == ret);

  ret = minus_slave->Init("conf/minus_slave/", &CalcService, NULL, minus_slave, end);
  assert(true == ret);

  client_handle.push_back(std::make_pair(ClientHandler, 1));
  ret = client->Init("conf/client/", NULL, &client_handle, client, end);
  assert(true == ret);

  ret = agent->Start() && plus_master->Start() && plus_slave->Start() && 
        minus_master->Start() && minus_slave->Start();
  assert(true == ret);

  ret = client->Start();
  assert(true == ret);

  delete [] servers;
  return 0;
}
