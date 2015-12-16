#include "magneto.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kNumClients=50;
static const size_t kNumReqs=500000;
static const int8_t kInput=10;
int num_reqs=kNumReqs;
int num_finished_clients=kNumClients;
int succ=0;
bool end=false;

void ClientHandler(void* args) {
  Timer timer;
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);
  while ( __sync_sub_and_fetch(&num_reqs, 1) >= 0 ) {
    client.Write("agent", magneto::Buf(Slice(RCAST<const char*>(&kInput), 1), NULL), 100);
    client.FreeTalks();
  }

  if (0 == __sync_sub_and_fetch(&num_finished_clients, 1)) {
    timer.Stop(true);
    sleep(5);
    printf("succ[%d] cost[%lu] qps[%f]\n", succ, timer.TimeUs(), kNumReqs*1000000.0/timer.TimeUs());
    end=true;
  }
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
    agent.Write("consumers", bufs, 100, errors);
  } else if (magneto::Protocol::kPing == protocol_read.GetCategory()) {
    agent.WriteBack(magneto::Buf(Slice("p", 1), NULL), 100);
  }
}

void ConsumerService(const magneto::ProtocolRead& protocol_read, void* /*args*/) {
  const magneto::ProtocolReadRapid& protocol_read_rapid = SCAST<const magneto::ProtocolReadRapid&>(protocol_read);
  if ( kInput == *(protocol_read_rapid.Data()) ) {
    __sync_add_and_fetch(&succ, 1);
  }
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
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_init_agent")

  ret = consumer0->Init("conf/consumer0/", &ConsumerService, NULL, consumer0, end);
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_init_plus_master")

  ret = consumer1->Init("conf/consumer1/", &ConsumerService, NULL, consumer1, end);
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_init_plus_slave")

  client_handle.push_back(std::make_pair(ClientHandler, kNumClients));
  ret = client->Init("conf/client/", NULL, &client_handle, client, end);
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_init_client")

  ret = agent->Start() && consumer0->Start() && consumer1->Start() && client->Start();
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_run_services")

  client->Stop();
  delete [] servers;
  return 0;

  ERROR_HANDLE:
  end=true;
  delete [] servers;
  return -1;
}
