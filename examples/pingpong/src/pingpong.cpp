#include "magneto.h"

namespace magneto {
LOGGER_IMPL(magneto_logger, "magneto")
}

bool end=false;

void PingHandler(const magneto::ProtocolRead& /*protocol_read*/, void* args) {
  RCAST<magneto::Magneto*>(args)->WriteBack(magneto::Buf(magneto::Slice("p", 1), NULL), 100);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  static const size_t kNumReqs=5000;
  size_t succ=0;
  magneto::ProtocolRead* response;
  magneto::Timer timer;
  for (size_t i=0; i<kNumReqs; ++i) {
    int ret = client.SimpleTalk("downstream", magneto::Buf(magneto::Slice("p", 1), NULL), 100, response);
    if (magneto::ErrorNo::kOk == ret) ++succ;
    client.FreeTalks();
  }
  timer.Stop(true);
  printf("succ[%lu] cost[%lu]\n", succ, timer.TimeUs());
  end=true;
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  magneto::Magneto* server = new magneto::Magneto;
  magneto::Magneto* client = new magneto::Magneto;
  magneto::RoutineItems client_handle;

  /* init server */
  int ret = server->Init("conf/confs_server/", &PingHandler, NULL, server, end);
  MAG_FAIL_HANDLE_FATAL_LOG(magneto::magneto_logger, !ret, "fail_init_server")

  /* init client */
  client_handle.push_back(std::make_pair(ClientHandler, 100));
  ret = client->Init("conf/confs_client/", NULL, &client_handle, client, end);
  MAG_FAIL_HANDLE_FATAL_LOG(magneto::magneto_logger, !ret, "fail_init_client")

  delete client; delete server;
  return 0;

  ERROR_HANDLE:
  end=true;
  delete client; delete server;
  return -1;
}
