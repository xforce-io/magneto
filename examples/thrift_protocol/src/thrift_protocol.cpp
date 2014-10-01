#include "magneto.h"
#include "thrift_helper.h"
#include "gen-cpp/PingPongTest.h"

namespace magneto {
LOGGER_IMPL(magneto, "magneto")
}

bool end=false;

void Service(const magneto::ProtocolRead& protocol_read, void* args) {
  const magneto::ProtocolReadRapid& protocol_read_rapid = 
    SCAST<const magneto::ProtocolReadRapid&>(protocol_read);
  magneto::Magneto& service = *RCAST<magneto::Magneto*>(args);
  app::PingPongTest_PingPong_args ping;
  BufToThrift<app::PingPongTest_PingPong_args>(
      protocol_read_rapid.Buf(), 
      protocol_read_rapid.Len(),
      ping);

  app::PingPongTest_PingPong_result pong;
  pong.__isset.success = true;
  pong.success.token = 2 * ping.ping.token;

  std::string out;
  ThriftToBuf<app::PingPongTest_PingPong_result>(pong, out);
  service.WriteBack(std::make_pair(out.data(), out.size()), 100);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  static const size_t kNumReqs=5000;
  size_t succ=0;
  magneto::ProtocolRead* response;
  magneto::Timer timer;
  app::PingPongTest_PingPong_args ping;
  ping.__isset.ping = true;
  ping.ping.token = 1;

  app::PingPongTest_PingPong_result pong;
  std::string out;
  ThriftToBuf<app::PingPongTest_PingPong_args>(ping, out);
  for (size_t i=0; i<kNumReqs; ++i) {
    client.SimpleTalk("downstream", std::make_pair(out.data(), out.size()), 100, response);
    BufToThrift<app::PingPongTest_PingPong_result>(
        response->Buf(), 
        response->Len(), 
        pong);

    if (2 == pong.success.token) ++succ;

    client.FreeTalks();
  }
  timer.Stop(true);
  printf("succ[%lu] cost[%lu]\n", succ, timer.TimeUs());
  end=true;
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  magneto::Magneto* service = new magneto::Magneto;
  magneto::Magneto* client = new magneto::Magneto;
  magneto::Magneto::RoutineItems client_handle;

  // init service
  int ret = service->Init("conf/confs_server/", &Service, NULL, service, end);
  MAG_FAIL_HANDLE_FATAL_LOG(magneto::magneto, !ret, "fail_init_server")

  // init client
  client_handle.push_back(std::make_pair(ClientHandler, 100));
  ret = client->Init("conf/confs_client/", NULL, &client_handle, client, end);
  MAG_FAIL_HANDLE_FATAL_LOG(magneto::magneto, !ret, "fail_init_client")

  delete client; delete service;
  return 0;

  ERROR_HANDLE:
  end=true;
  delete client; delete service;
  return -1;
}
