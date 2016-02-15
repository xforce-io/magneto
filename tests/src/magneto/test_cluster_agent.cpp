#include "gtest/gtest.h"
#include "../../../src/magneto.h"
#include "../../../src/confs/confs.h"
#include <sstream>

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

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);

  int8_t kInput=5;
  magneto::ProtocolRead* protocol_read;
  Timer timer;
  while ( __sync_sub_and_fetch(&num_reqs, 1) >= 0 ) {
    int ret = client.SimpleTalk("agent", magneto::Buf(Slice(RCAST<const char*>(&kInput), 1), NULL), kTimeoutMs, protocol_read);
    const magneto::ProtocolReadRapid& protocol_read_rapid = 
      SCAST<const magneto::ProtocolReadRapid&>(*protocol_read);
    if (magneto::ErrorNo::kOk == ret && kInput*2 == *(protocol_read_rapid.Data())) {
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

int main(int argc, char** argv) {
  LOGGER_SYS_INIT("../conf/log.conf");
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestClusterAgent, all) {
  static const size_t kNumServers=6;

  magneto::Magneto* servers = new magneto::Magneto [kNumServers];
  magneto::Magneto* client = &(servers[0]);
  magneto::Magneto* agent = &(servers[1]);
  magneto::Magneto* plus_master = &(servers[2]);
  magneto::Magneto* plus_slave = &(servers[3]);
  magneto::Magneto* minus_master = &(servers[4]);
  magneto::Magneto* minus_slave = &(servers[5]);
  magneto::RoutineItems client_handle;

  std::string conf_prefix = "../conf/magneto/test_cluster_agent/";

  int ret = agent->Init(conf_prefix+"/agent/", &AgentService, NULL, agent, end);
  ASSERT_TRUE(ret);

  ret = plus_master->Init(conf_prefix+"/plus_master/", &CalcService, NULL, plus_master, end);
  ASSERT_TRUE(ret);

  ret = plus_slave->Init(conf_prefix+"/plus_slave/", &CalcService, NULL, plus_slave, end);
  ASSERT_TRUE(ret);

  ret = minus_master->Init(conf_prefix+"/minus_master/", &CalcService, NULL, minus_master, end);
  ASSERT_TRUE(ret);

  ret = minus_slave->Init(conf_prefix+"/minus_slave/", &CalcService, NULL, minus_slave, end);
  ASSERT_TRUE(ret);

  client_handle.push_back(std::make_pair(ClientHandler, kNumClients));
  ret = client->Init(conf_prefix+"/client/", NULL, &client_handle, client, end);
  ASSERT_TRUE(ret);

  ret = agent->Start() && plus_master->Start() && plus_slave->Start() && 
        minus_master->Start() && minus_slave->Start();
  ASSERT_TRUE(ret);

  ret = client->Start();
  ASSERT_TRUE(ret);

  client->Stop();
  delete [] servers;
}
