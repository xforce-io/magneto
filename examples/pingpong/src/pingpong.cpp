#include "magneto.h"

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kTimeoutMs=500;
bool end=false;

void PingHandler(const magneto::ProtocolRead& /*protocol_read*/, void* args) {
  RCAST<magneto::Magneto*>(args)->WriteBack(magneto::Buf(Slice("p", 1), NULL), kTimeoutMs);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);
  magneto::ProtocolRead* response;
  int ret = client.SimpleTalk("downstream", magneto::Buf(Slice("p", 1), NULL), kTimeoutMs, response);
  assert(magneto::ErrorNo::kOk == ret);
  client.FreeTalks();
  end=true;
}

int main() {
  LOGGER_SYS_INIT("conf/log.conf")

  magneto::Magneto* server = new magneto::Magneto;
  magneto::Magneto* client = new magneto::Magneto;
  magneto::RoutineItems client_handle;

  /* init server */
  int ret = server->Init("conf/confs_server/", &PingHandler, NULL, server, end);
  assert(true == ret);

  /* init client */
  client_handle.push_back(std::make_pair(ClientHandler, 1));
  ret = client->Init("conf/confs_client/", NULL, &client_handle, client, end);
  assert(true == ret);

  ret = server->Start() && client->Start();
  assert(true == ret);

  client->Stop();

  delete client; delete server;
  return 0;
}
