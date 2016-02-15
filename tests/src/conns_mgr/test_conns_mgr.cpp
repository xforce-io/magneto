#include "gtest/gtest.h"
#include "../../../src/conns_mgr/conns_mgr.h"

using namespace xforce;
using namespace xforce::magneto;

namespace xforce {
LOGGER_IMPL(xforce_logger, "magneto")
}

int main(int argc, char** argv) {
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class TestConnsMgr: public ::testing::Test {
 private:
  typedef std::vector<Remote> Remotes;
  
 private:
  static const std::string kConfPath;
  
 protected:
  TestConnsMgr();
  virtual ~TestConnsMgr() {}

  virtual void SetUp();
  virtual void TearDown() {}
 
 private: 
  void InitConfNormal_();
  ConfServices* InitConfServices_();
  ConfServices* InitConfServices1_();
  void TestCase0_(
      const Service& service,
      ConnsMgr& conns_mgr, 
      const Remotes& remotes, 
      const Remotes& fail_remotes);
 
  void TestCase1_(
      const Service& service,
      ConnsMgr& conns_mgr, 
      const Remotes& remotes, 
      const Remotes& fail_remotes);

  void TestCase2_(
      const Service& service,
      ConnsMgr& conns_mgr, 
      const Remotes& remotes, 
      const Remotes& fail_remotes);

  void TestCase3_(
      const Service& service,
      ConnsMgr& conns_mgr, 
      const Remotes& remotes, 
      const Remotes& fail_remotes);

 private:
  ConfNormal conf_normal_; 
  ConnsMgr conns_mgr_;
};

const std::string TestConnsMgr::kConfPath = "data/test_conf_normals";

TestConnsMgr::TestConnsMgr() :
  conns_mgr_(2) {}

void TestConnsMgr::SetUp() {
  system("rm -rf data/test_conns_mgr/; mkdir -p data/test_conns_mgr");
  InitConfNormal_();
}

void TestConnsMgr::InitConfNormal_() {
  system("echo '"
    "{ "
      "\"protocol\" : \"rapid\", "
      "\"num_schedulers\" : 2, "
      "\"num_agents\" : 2, "
      "\"client_keepalive_sec\" : 100, "
      "\"max_num_clients_keepalive\" : 1000, "
      "\"long_conn_keepalive_sec\" : 120, "
      "\"ctimeo_ms\" : 120, "
      "\"wtimeo_ms\" : 120, "
      "\"rtimeo_ms\" : 120, "
      "\"listen_addrs\" : {"
        "\"minus|rapid\" : \"0.0.0.0:1234\","
        "\"plus|rapid\" : \"1.1.1.1:4321\""
      "},"
      "\"size_scheduler_mailbox\" : 1000000, "
      "\"size_agent_mailbox\" : 1000000, "
      "\"size_stack\" : 1000000, "
      "\"low_latency\" : 1, "
      "\"default_long_conns\" : 50, "
      "\"default_weight\" : 50"
    "}'"
  " > data/test_conf_normals/normal.conf");
  ASSERT_TRUE(conf_normal_.Init(kConfPath));
}

ConfServices* TestConnsMgr::InitConfServices_() {
  ConfServices* conf_services = new ConfServices;
  system("echo '"
    "{"
      "\"services_sets\" : {},"
      "\"services\" : {"
        "\"moon|ping\" : {"
          "\"strategy\":\"weight\","
          "\"remotes\": ["
            "{"
              "\"addr\" :\"0.0.0.0:6610\","
              "\"long_conns\" : 60"
            "},{"
              "\"addr\" :\"0.0.0.0:6620\","
              "\"weight\" : 100"
           "},{"
              "\"addr\" :\"0.0.0.0:6630\","
              "\"weight\" : 150"
            "}" 
          "]"
        "}" 
      "}"
    "}"  
  "' > data/test_conf_normals/services.conf");
  assert(conf_services->Init(kConfPath, conf_normal_));
  return conf_services;
}

ConfServices* TestConnsMgr::InitConfServices1_() {
  ConfServices* conf_services = new ConfServices;
  system("echo '"
    "{"
      "\"services_sets\" : {},"
      "\"services\" : {"
        "\"moon|rapid\" : {"
          "\"strategy\":\"weight\","
          "\"remotes\": ["
            "{"
              "\"addr\" :\"0.0.0.0:6610\","
              "\"long_conns\" : 60"
            "},{"
              "\"addr\" :\"0.0.0.0:6630\","
              "\"weight\" : 150"
            "}" 
          "]"
        "}" 
      "}"
    "}"  
  "' > data/test_conf_normals/services.conf");
  assert(conf_services->Init(kConfPath, conf_normal_));
  return conf_services;
}

void TestConnsMgr::TestCase0_(
    const Service& service,
    ConnsMgr& conns_mgr, 
    const Remotes& remotes, 
    const Remotes& fail_remotes) {
  const static size_t kTimesTest=100000;

  ASSERT_EQ(0, conns_mgr.unhealthy_remotes_.size());
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[0]));
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[1]));
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[2]));

  std::vector<size_t> chooses;
  chooses.resize(remotes.size());
  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }

  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteWeight_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }

  ASSERT_TRUE(kTimesTest == chooses[0] + chooses[1] + chooses[2]);
  double ratio = chooses[2] * 1.0 / chooses[0];
  ASSERT_TRUE(ratio>2.8 && ratio<3.2);

  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }
  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteNormal_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }
  ASSERT_TRUE(kTimesTest==chooses[0]);
}

void TestConnsMgr::TestCase1_(
    const Service& service,
    ConnsMgr& conns_mgr, 
    const Remotes& remotes, 
    const Remotes& fail_remotes) {
  const static size_t kTimesTest=100000;

  ASSERT_EQ(1, conns_mgr.unhealthy_remotes_.size());
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[0]));
  ASSERT_TRUE(!conns_mgr.IsHealthy_(remotes[1]));
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[2]));

  std::vector<size_t> chooses;
  chooses.resize(remotes.size());
  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }

  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteWeight_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }

  ASSERT_TRUE(kTimesTest == chooses[0] + chooses[1] + chooses[2]);
  double ratio = chooses[2] * 1.0 / chooses[0];
  ASSERT_TRUE(ratio>2.8 && ratio<3.2);
  ratio = chooses[1] * 1.0 / chooses[0];
  ASSERT_TRUE(ratio>0.8 && ratio<1.2);
}

void TestConnsMgr::TestCase2_(
    const Service& service,
    ConnsMgr& conns_mgr, 
    const Remotes& remotes, 
    const Remotes& fail_remotes) {
  const static size_t kTimesTest=100000;

  ASSERT_EQ(2, conns_mgr.unhealthy_remotes_.size());
  ASSERT_TRUE(!conns_mgr.IsHealthy_(remotes[0]));
  ASSERT_TRUE(!conns_mgr.IsHealthy_(remotes[1]));
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[2]));

  std::vector<size_t> chooses;
  chooses.resize(remotes.size());
  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }

  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteWeight_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }

  ASSERT_TRUE(kTimesTest == chooses[0] + chooses[1] + chooses[2]);
  double ratio = chooses[2] * 1.0 / chooses[0];
  ASSERT_TRUE(ratio>5.8 && ratio<6.2);
  ratio = chooses[1] * 1.0 / chooses[0];
  ASSERT_TRUE(ratio>0.8 && ratio<1.2);

  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }
  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteNormal_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }
  ASSERT_TRUE(0 == chooses[0] && 0 == chooses[1] && kTimesTest == chooses[2]);
}

void TestConnsMgr::TestCase3_(
    const Service& service,
    ConnsMgr& conns_mgr, 
    const Remotes& remotes, 
    const Remotes& fail_remotes) {
  const static size_t kTimesTest=100000;

  ASSERT_EQ(1, conns_mgr.unhealthy_remotes_.size());
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[0]));
  ASSERT_TRUE(!conns_mgr.IsHealthy_(remotes[1]));
  ASSERT_TRUE(conns_mgr.IsHealthy_(remotes[2]));

  std::vector<size_t> chooses;
  chooses.resize(remotes.size());
  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }

  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteWeight_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }

  ASSERT_TRUE(0==chooses[2]);
  double ratio = chooses[1] * 1.0 / chooses[0];
  ASSERT_TRUE(ratio>0.8 && ratio<1.2);

  for (size_t i=0; i < chooses.size(); ++i) {
    chooses[i] = 0;
  }
  for (size_t i=0; i<kTimesTest; ++i) {
    const Remote* remote = conns_mgr_.GetRemoteNormal_(service, fail_remotes);
    for (size_t i=0; i < chooses.size(); ++i) {
      if (remotes[i].name == remote->name) {
        ++chooses[i];
      }
    }
  }
  ASSERT_TRUE(kTimesTest == chooses[0]);
}

TEST_F(TestConnsMgr, all) {
  ConfServices* conf_services = InitConfServices_();
  const ConfServices::Remotes& all_remotes = conf_services->GetRemotes();
  ConfServices::Remotes::const_iterator iter = all_remotes.begin();
  Remotes remotes;
  remotes.push_back(all_remotes.find("0.0.0.0:6610")->second);
  remotes.push_back(all_remotes.find("0.0.0.0:6620")->second);
  remotes.push_back(all_remotes.find("0.0.0.0:6630")->second);

  ConnsMgr::Remotes fail_remotes;

  conns_mgr_.ConfigRemotes(all_remotes);

  conns_mgr_.ReportStatus(remotes[0].name, true);
  conns_mgr_.ReportStatus(remotes[1].name, true);
  conns_mgr_.ReportStatus(remotes[2].name, true);
  TestCase0_(*(conf_services->GetServices().begin()->second), conns_mgr_, remotes, fail_remotes);

  conns_mgr_.ReportStatus(remotes[0].name, true);
  conns_mgr_.ReportStatus(remotes[1].name, false);
  conns_mgr_.ReportStatus(remotes[2].name, true);
  TestCase1_(*(conf_services->GetServices().begin()->second), conns_mgr_, remotes, fail_remotes);

  conns_mgr_.ReportStatus(remotes[0].name, false);
  conns_mgr_.ReportStatus(remotes[1].name, false);
  conns_mgr_.ReportStatus(remotes[2].name, true);
  TestCase2_(*(conf_services->GetServices().begin()->second), conns_mgr_, remotes, fail_remotes);

  conns_mgr_.ReportStatus(remotes[0].name, true);
  conns_mgr_.ReportStatus(remotes[1].name, true);
  conns_mgr_.ReportStatus(remotes[2].name, true);
  fail_remotes.push_back(remotes[2]);
  TestCase3_(*(conf_services->GetServices().begin()->second), conns_mgr_, remotes, fail_remotes);

  ConfServices* conf_services1 = InitConfServices1_();
  conns_mgr_.ConfigRemotes(conf_services1->GetRemotes());
  ASSERT_EQ(2, conns_mgr_.conns_.Size());
  ASSERT_EQ(0, conns_mgr_.unhealthy_remotes_.size());

  delete conf_services1;
  delete conf_services;
}
