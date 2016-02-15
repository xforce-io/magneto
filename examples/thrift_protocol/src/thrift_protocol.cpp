#include "magneto.h"
#include "gen-cpp/PingPongTest.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

bool end=false;

void Service(const magneto::ProtocolRead& protocol_read, void* args) {
  const magneto::ProtocolReadThrift& protocol_read_thrift = 
    SCAST<const magneto::ProtocolReadThrift&>(protocol_read);
  magneto::Magneto& service = *RCAST<magneto::Magneto*>(args);
  app::PingPongTest_PingPong_args ping;
  magneto::BufToThrift<app::PingPongTest_PingPong_args>(
      protocol_read_thrift.Data(), 
      protocol_read_thrift.Size(),
      ping);

  app::PingPongTest_PingPong_result pong;
  pong.__isset.success = true;
  pong.success.token = 2 * ping.ping.token;

  magneto::ProtocolWriteThrift::Params params(
      protocol_read_thrift.Fn(), 
      magneto::ProtocolWriteThrift::kReply, 
      protocol_read_thrift.SeqId() + 1);

  std::string out;
  magneto::ThriftToBuf<app::PingPongTest_PingPong_result>(pong, out);
  service.WriteBack(magneto::Buf(Slice(out.data(), out.size()), &params), 100);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  magneto::ProtocolRead* response;
  Timer timer;
  app::PingPongTest_PingPong_args ping;
  ping.__isset.ping = true;
  ping.ping.token = 1;

  app::PingPongTest_PingPong_result pong;
  std::string out;
  magneto::ThriftToBuf<app::PingPongTest_PingPong_args>(ping, out);
  magneto::ProtocolWriteThrift::Params params(
      "PingPong", 
      magneto::ProtocolWriteThrift::kRequest, 
      1);
  int ret = client.SimpleTalk("downstream", magneto::Buf(Slice(out.data(), out.size()), &params), 1000, response);
  if (magneto::ErrorNo::kOk == ret) {
    magneto::BufToThrift<app::PingPongTest_PingPong_result>(
        response->Data(), 
        response->Size(), 
        pong);
    assert(2 == pong.success.token);
  }
  client.FreeTalks();
  end=true;
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  magneto::Magneto* service = new magneto::Magneto;
  magneto::Magneto* client = new magneto::Magneto;
  magneto::RoutineItems client_handle;

  // init service
  int ret = service->Init("conf/confs_server/", &Service, NULL, service, end);
  assert(true == ret);

  // init client
  client_handle.push_back(std::make_pair(ClientHandler, 100));
  ret = client->Init("conf/confs_client/", NULL, &client_handle, client, end);
  assert(true == ret);

  ret = service->Start() && client->Start();
  assert(true == ret);

  delete client; delete service;
  return 0;
}
