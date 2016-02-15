#include "gtest/gtest.h"
#include "../../../src/magneto.h"
#include "../../../src/confs/confs.h"
#include <sstream>

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

using namespace xforce;

static const size_t kNumServers=4;
static const size_t kNumReqs=100000;
static const size_t kTimeoutMs=500;
int num_reqs=kNumReqs;
int succ=0;
bool end=false;
static const int kStartListenPort = 6629;

void MakeConfsForServers(size_t num_servers) {
  for (size_t i=0; i<num_servers; ++i) {
    std::stringstream ss;
    ss << "mkdir -p data/test_reload_confs/conf_server" << i;
    system(ss.str().c_str());

    ss.str("");
    ss << "echo '"
      "{ "
        "\"num_schedulers\" : 2, "
        "\"num_agents\" : 2, "
        "\"client_keepalive_sec\" : 100, "
        "\"max_num_clients_keepalive\" : 1000, "
        "\"long_conn_keepalive_sec\" : 120, "
        "\"ctimeo_ms\" : 120, "
        "\"wtimeo_ms\" : 120, "
        "\"rtimeo_ms\" : 120, "
        "\"listen_addrs\" : {"
          "\"minus|ping\" : \"0.0.0.0:" << (kStartListenPort+i) << "\""
        "},"
        "\"size_scheduler_mailbox\" : 1000000, "
        "\"size_agent_mailbox\" : 1000000, "
        "\"size_stack\" : 1000000, "
        "\"low_latency\" : 1, "
        "\"default_long_conns\" : 50, "
        "\"default_weight\" : 50"
      "}'"
    " > data/test_reload_confs/conf_server" << i << "/normal.conf";
    system(ss.str().c_str());
  }
}

void MakeConfsForClient(size_t num_servers) {
  std::stringstream ss;
  ss << "mkdir -p data/test_reload_confs/conf_client";
  system(ss.str().c_str());

  ss.str("");
  ss << "echo '"
    "{ " 
      "\"services_sets\" : {},"
      "\"services\" :"
        "{"
           "\"downstream|ping\" :"
            "{"
              "\"retry\" : 1,"
              "\"remotes\": "
                "[";

  for (size_t i=0; i<num_servers-1; ++i) {
    ss << "{ \"addr\" :\"127.0.0.1:" 
        << (kStartListenPort + i) 
        << "\" },";
  }
  ss << "{ \"addr\" :\"127.0.0.1:" 
      << (kStartListenPort + num_servers - 1) 
      << "\" }";
  ss <<         "]"
            "}"
        "}"
    "}'"
  " > data/test_reload_confs/conf_client/services.conf";
  system(ss.str().c_str());

  ss.str("");
  ss << "echo '"
    "{ "
      "\"num_schedulers\" : 2, "
      "\"num_agents\" : 2, "
      "\"client_keepalive_sec\" : 100, "
      "\"max_num_clients_keepalive\" : 1000, "
      "\"long_conn_keepalive_sec\" : 120, "
      "\"ctimeo_ms\" : 120, "
      "\"wtimeo_ms\" : 120, "
      "\"rtimeo_ms\" : 120, "
      "\"listen_addrs\" : {},"
      "\"size_scheduler_mailbox\" : 1000000, "
      "\"size_agent_mailbox\" : 1000000, "
      "\"size_stack\" : 1000000, "
      "\"low_latency\" : 1, "
      "\"default_long_conns\" : 50, "
      "\"default_weight\" : 50"
    "}'"
  " > data/test_reload_confs/conf_client/normal.conf";
  system(ss.str().c_str());
}

void PingHandler(const magneto::ProtocolRead& /*protocol_read*/, void* args) {
  RCAST<magneto::Magneto*>(args)->WriteBack(magneto::Buf(Slice("p", 1), NULL), kTimeoutMs);
}

void ClientHandler(void* args) {
  magneto::Magneto& client = *RCAST<magneto::Magneto*>(args);
  magneto::ProtocolRead* protocol_read;

  int ret = client.SimpleTalk(
      "downstream", 
      "127.0.0.1:6629",
      magneto::Buf(Slice("p", 1), NULL), 
      kTimeoutMs, 
      protocol_read);
  assert(ret == magneto::ErrorNo::kOk);

  ret = client.SimpleTalk(
      "downstream", 
      "127.0.0.1:6630",
      magneto::Buf(Slice("p", 1), NULL), 
      kTimeoutMs, 
      protocol_read);
  assert(ret == magneto::ErrorNo::kNotFound);

  MakeConfsForClient(2);
  sleep(magneto::Confs::kMinReloadIntervalSec + 1);

  ret = client.SimpleTalk(
      "downstream", 
      "127.0.0.1:6630",
      magneto::Buf(Slice("p", 1), NULL), 
      kTimeoutMs, 
      protocol_read);
  assert(ret == magneto::ErrorNo::kOk);
  
  end=true;
}

int main(int argc, char** argv) {
  LOGGER_SYS_INIT("../conf/log.conf");
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestReloadConfs, all) {
  int ret; 

  system("rm -rf data/test_reload_confs/; mkdir -p data/test_reload_confs");
  MakeConfsForServers(kNumServers);
  MakeConfsForClient(1);

  magneto::Magneto* server = new magneto::Magneto [kNumServers];
  magneto::Magneto* client = new magneto::Magneto;
  magneto::RoutineItems client_handle;

  /* init server */
  for (size_t i=0; i<kNumServers; ++i) {
    std::stringstream ss;
    ss << "data/test_reload_confs/conf_server" << i;

    ret = server[i].Init(ss.str(), &PingHandler, NULL, &(server[i]), end);
    ASSERT_TRUE(ret);
  }

  /* init client */
  client_handle.push_back(std::make_pair(ClientHandler, 1));
  ret = client->Init("data/test_reload_confs/conf_client/", NULL, &client_handle, client, end);
  ASSERT_TRUE(ret);

  for (size_t i=0; i<kNumServers; ++i) {
    ret = server[i].Start();
    ASSERT_TRUE(ret);
  }

  ret = client->Start();
  ASSERT_TRUE(ret);

  client->Stop();
  delete [] server;
}
