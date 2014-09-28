#include "gtest/gtest.h"
#include "../../../src/confs/conf_services.h"

using namespace magneto;

namespace magneto {
LOGGER_IMPL(magneto, "magneto")
}

int main(int argc, char** argv) {
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class TestConf: public ::testing::Test {
 protected:
  virtual ~TestConf() {};

  virtual void SetUp() {
    system("rm -rf data/test_conf_normals/; mkdir -p data/test_conf_normals");
    TestConfNormal_();
    TestConfService_();
  }

  virtual void TearDown() {}
 
 private:
  void TestConfNormal_();
  void TestConfService_();
 
 private:
  ConfNormal conf_normal_; 
  ConfServices conf_services_;
};

void TestConf::TestConfNormal_() {
  system("echo '"
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
        "\"minus|rapid\" : \"1.1.1.1:4321\""
      "},"
      "\"size_scheduler_mailbox\" : 1000000, "
      "\"size_agent_mailbox\" : 1000000, "
      "\"size_stack\" : 1000000, "
      "\"low_latency\" : 1, "
      "\"default_long_conns\" : 50, "
      "\"default_weight\" : 50"
    "}'"
  " > data/test_conf_normals/normal.conf");

  ASSERT_TRUE(conf_normal_.Init("data/test_conf_normals"));
  ASSERT_TRUE(100 == conf_normal_.GetClientKeepaliveSec());
  ASSERT_TRUE(120 == conf_normal_.GetLongConnKeepaliveSec());
  ASSERT_TRUE(1 == conf_normal_.GetListenAddrs().size());
  ASSERT_TRUE(htons(4321) == conf_normal_.GetListenAddrs()[0].addr.addr.sin_port);
  ASSERT_TRUE(Protocol::kRapid == conf_normal_.GetListenAddrs()[0].category);
}

void TestConf::TestConfService_() {
  system("rm -rf data/test_conf_services/; mkdir -p data/test_conf_services");
  system("echo '"
    "{"
      "\"services_sets\" : { "
        "\"set0\" : [\"moon\",\"mars\" ],"
        "\"set1\" : [\"sun\",\"uranus\" ]"
      "},"
      "\"services\" : {"
        "\"moon|rapid\" : {"
          "\"strategy\":\"weight\","
          "\"remotes\": ["
            "{"
              "\"addr\" :\"0.0.0.0:6610\","
              "\"long_conns\" : 60"
            "},{"
              "\"addr\" :\"0.0.0.0:6620\","
              "\"weight\" : 100"
            "}" 
          "]"
        "},"
        "\"mars|ping\" : {"
          "\"strategy\":\"normal\","
          "\"remotes\": ["
            "{"
              "\"addr\" :\"1.0.0.0:6610\","
              "\"wtimeo_ms\" : 120,"
              "\"rtimeo_ms\" : 100"
            "},{"
              "\"addr\" :\"1.0.0.0:6620\","
              "\"long_conns\" : 0,"
              "\"weight\" : 10,"
              "\"wtimeo_ms\" : 100"
            "}"
          "]"
        "}," 
        "\"sun|ping\" : {"
          "\"strategy\":\"weight\","
          "\"remotes\": ["
            "{ \"addr\" :\"2.0.0.0:6610\" },"
            "{ \"addr\" :\"2.0.0.0:6620\" }"
          "]"
        "},"
        "\"uranus|redis\" : {"
          "\"strategy\":\"weight\","
          "\"remotes\": ["
            "{ \"addr\" :\"3.0.0.0:6610\" }, "
            "{ \"addr\" :\"3.0.0.0:6620\" } "
          "]"
        "}"
      "}"
    "}"  
  "' > data/test_conf_services/services.conf");

  bool ret = conf_services_.Init("data/test_conf_services", conf_normal_);
  ASSERT_TRUE(ret);

  //test services_sets
  ConfServices::ServicesSets::const_iterator iter = conf_services_.GetServicesSets().find("set1");
  ASSERT_EQ(2, iter->second->size());
  ASSERT_TRUE("uranus" == (*iter->second)[1]->name);

  //test_services
  ConfServices::Services::const_iterator iter_2 = conf_services_.services_.find("mars");
  ASSERT_TRUE(conf_services_.services_.end() != iter_2);
  ASSERT_EQ(2, iter_2->second->remotes.size());
  ASSERT_EQ(Protocol::kPing, iter_2->second->protocol_category);
  ASSERT_EQ(ServiceStrategy::kNormal, iter_2->second->service_strategy);
  ASSERT_EQ(10, iter_2->second->remotes[1].weight);
  ASSERT_EQ(120, iter_2->second->remotes[1].rtimeo_ms);
  ASSERT_EQ(100, iter_2->second->remotes[1].wtimeo_ms);
  ASSERT_EQ(60, iter_2->second->weight_all);

  //test addrs
  ASSERT_EQ(8, conf_services_.remotes_.size());
}

TEST_F(TestConf, all) {}
