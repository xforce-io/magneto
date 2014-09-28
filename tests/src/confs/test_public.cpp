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

class TestPublic: public ::testing::Test {
 protected:
  TestPublic() {};
  virtual ~TestPublic() {};
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(TestPublic, test_parse_service_tag) {
  std::string service_name;
  Protocol::Category category;

  /*
   * positive
   */
  ASSERT_TRUE(ConfHelper::ParseServiceTag("hello|rapid", service_name, category));
  ASSERT_TRUE(service_name == "hello");
  ASSERT_TRUE(category == Protocol::kRapid);

  /*
   * passive
   */
  ASSERT_FALSE(ConfHelper::ParseServiceTag("|rapid", service_name, category));
  ASSERT_FALSE(ConfHelper::ParseServiceTag("rapid|", service_name, category));
  ASSERT_FALSE(ConfHelper::ParseServiceTag("rapid", service_name, category));
  ASSERT_FALSE(ConfHelper::ParseServiceTag("hello|unknow", service_name, category));
}
