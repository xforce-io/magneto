#include "gtest/gtest.h"
#include "../../../src/magneto.h"
#include "../../../src/confs/confs.h"
#include <sstream>

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kNumClients=10;
static const size_t kNumReqs=50000;
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
    assert(succ == kNumReqs);
  }
}

int main(int argc, char** argv) {
  LOGGER_SYS_INIT("../conf/log.conf");
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestClusterAgent, all) {
  std::string conf_prefix = "../conf/magneto/test_pingpong/";

  magneto::Magneto* server = new magneto::Magneto;
  magneto::Magneto* client = new magneto::Magneto;
  magneto::RoutineItems client_handle;

  /* init server */
  int ret = server->Init(conf_prefix+"/confs_server/", &PingHandler, NULL, server, end);
  ASSERT_TRUE(ret);

  /* init client */
  client_handle.push_back(std::make_pair(ClientHandler, kNumClients));
  ret = client->Init(conf_prefix+"/confs_client/", NULL, &client_handle, client, end);
  ASSERT_TRUE(ret);

  ret = server->Start() && client->Start();
  ASSERT_TRUE(ret);

  delete client; delete server;
}
