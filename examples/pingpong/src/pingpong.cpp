#include "magneto.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kNumClients=50;
static const size_t kNumReqs=100000;
static const size_t kTimeoutMs=500;
int num_reqs=kNumReqs;
int num_finished_clients=kNumClients;
int succ=0;
bool end=false;

void PingHandler(const magneto::ProtocolRead& /*protocol_read*/, void* args) {
  RCAST<magneto::Magneto*>(args)->WriteBack(magneto::Buf(Slice("p", 1), NULL), kTimeoutMs);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  magneto::ProtocolRead* response;
  Timer timer;
  while ( __sync_sub_and_fetch(&num_reqs, 1) >= 0 ) {
    int ret = client.SimpleTalk("downstream", magneto::Buf(Slice("p", 1), NULL), kTimeoutMs, response);
    if (magneto::ErrorNo::kOk == ret) {
      __sync_fetch_and_add(&succ, 1);
    }
    client.FreeTalks();
  }

  if (0 == __sync_sub_and_fetch(&num_finished_clients, 1)) {
    end=true;
    timer.Stop(true);
    printf("all[%lu] succ[%d] cost[%lu] qps[%f]\n", 
            kNumReqs, succ, timer.TimeUs(), kNumReqs*1000000.0/timer.TimeUs());
  }
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  magneto::Magneto* server = new magneto::Magneto;
  magneto::Magneto* client = new magneto::Magneto;
  magneto::RoutineItems client_handle;

  /* init server */
  int ret = server->Init("conf/confs_server/", &PingHandler, NULL, server, end);
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_init_server")

  /* init client */
  client_handle.push_back(std::make_pair(ClientHandler, kNumClients));
  ret = client->Init("conf/confs_client/", NULL, &client_handle, client, end);
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_init_client")

  ret = server->Start() && client->Start();
  XFC_FAIL_HANDLE_FATAL_LOG(xforce_logger, !ret, "fail_run_services")

  delete client; delete server;
  return 0;

  ERROR_HANDLE:
  end=true;
  delete client; delete server;
  return -1;
}
