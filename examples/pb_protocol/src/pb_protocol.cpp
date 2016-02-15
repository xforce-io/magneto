#include "magneto.h"
#include "sample.pb.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

bool end=false;

void Service(const magneto::ProtocolRead& protocol_read, void* args) {
  const magneto::ProtocolReadProtobuf& protocol_read_protobuf = 
    SCAST<const magneto::ProtocolReadProtobuf&>(protocol_read);
  const ::Test::Msg* msg = RCAST<const ::Test::Msg*>(protocol_read_protobuf.GetMsg());
  magneto::Magneto& service = *RCAST<magneto::Magneto*>(args);
  service.WriteBack(
      magneto::Buf(Slice(RCAST<const char*>(msg), 0), RCAST<const void*>("Test.Msg")), 
      100);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  magneto::ProtocolRead* response;
  magneto::ProtocolWriteProtobuf protocol_write;
  ::Test::Msg msg;
  msg.set_userid(1);
  msg.set_name("2");
  msg.add_creattime(3);

  int ret = client.SimpleTalk(
      "downstream", 
      magneto::Buf(Slice(RCAST<char*>(&msg), 0), RCAST<const void*>("Test.Msg")), 
      1000, 
      response);
  if (magneto::ErrorNo::kOk == ret) {
    const magneto::ProtocolReadProtobuf& response_pb = 
      *SCAST<const magneto::ProtocolReadProtobuf*>(response);
    const ::Test::Msg* msg = RCAST<const ::Test::Msg*>(
        response_pb.GetMsg());
    assert(1 == msg->userid());
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
  client_handle.push_back(std::make_pair(ClientHandler, 10));
  ret = client->Init("conf/confs_client/", NULL, &client_handle, client, end);
  assert(true == ret);

  ret = service->Start() && client->Start();
  assert(true == ret);

  delete client; delete service;
  return 0;
}
