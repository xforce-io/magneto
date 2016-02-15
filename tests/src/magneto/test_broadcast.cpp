#include "gtest/gtest.h"
#include "../../../src/magneto.h"
#include "../../../src/confs/confs.h"
#include <sstream>

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kNumClients=1;
static const size_t kNumReqs=10000;
static const size_t kTimeoutMs=500;
static const int8_t kInput=10;
int num_reqs=kNumReqs;
int num_finished_clients=kNumClients;
int succ=0;
bool end=false;

void ClientHandler(void* args) {
  Timer timer;
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);
  while ( __sync_sub_and_fetch(&num_reqs, 1) >= 0 ) {
    int ret = client.Write("agent", magneto::Buf(Slice(RCAST<const char*>(&kInput), 1), NULL), kTimeoutMs);
    assert(ret == magneto::ErrorNo::kOk);
    client.FreeTalks();
  }

  if (0 == __sync_sub_and_fetch(&num_finished_clients, 1)) {
    end=true;
    size_t cont = 0;
    while (true) {
      printf("succ[%d]\n", succ);
      int prev_succ=succ;
      sleep(1);
      if (succ==prev_succ) {
        if (cont>1) {
          break;
        } else {
          ++cont;
        }
      }
    }
    timer.Stop(true);
    printf("all[%lu] succ[%d] cost[%lu] qps[%f]\n", 
            kNumReqs, succ, timer.TimeUs(), kNumReqs*1000000.0/timer.TimeUs());
    assert(succ == 2*kNumReqs);
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
    agent.Write("consumers", bufs, kTimeoutMs, errors);
  } else if (magneto::Protocol::kPing == protocol_read.GetCategory()) {
    agent.WriteBack(magneto::Buf(Slice("p", 1), NULL), kTimeoutMs);
  }
}

void ConsumerService(const magneto::ProtocolRead& protocol_read, void* /*args*/) {
  const magneto::ProtocolReadRapid& protocol_read_rapid = SCAST<const magneto::ProtocolReadRapid&>(protocol_read);
  if ( kInput == *(protocol_read_rapid.Data()) ) {
    __sync_add_and_fetch(&succ, 1);
  }
}

int main(int argc, char** argv) {
  LOGGER_SYS_INIT("../conf/log.conf");
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestBroadcast, all) {
  std::string conf_prefix = "../conf/magneto/test_broadcast/";

  static const size_t kNumServers=4;
  magneto::Magneto* servers = new magneto::Magneto [kNumServers];
  magneto::Magneto* client = &(servers[0]);
  magneto::Magneto* agent = &(servers[1]);
  magneto::Magneto* consumer0 = &(servers[2]);
  magneto::Magneto* consumer1 = &(servers[3]);
  magneto::RoutineItems client_handle;

  int ret = agent->Init(conf_prefix+"/agent/", &AgentService, NULL, agent, end);
  ASSERT_TRUE(ret);

  ret = consumer0->Init(conf_prefix+"/consumer0/", &ConsumerService, NULL, consumer0, end);
  ASSERT_TRUE(ret);

  ret = consumer1->Init(conf_prefix+"/consumer1/", &ConsumerService, NULL, consumer1, end);
  ASSERT_TRUE(ret);

  client_handle.push_back(std::make_pair(ClientHandler, kNumClients));
  ret = client->Init(conf_prefix+"/client/", NULL, &client_handle, client, end);
  ASSERT_TRUE(ret);

  ret = agent->Start() && consumer0->Start() && consumer1->Start() && client->Start();
  ASSERT_TRUE(ret);

  client->Stop();
  delete [] servers;

}
