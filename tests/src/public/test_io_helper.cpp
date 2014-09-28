#include "gtest/gtest.h"
#include "../../../src/public/io_helper.h"

using namespace magneto;

namespace magneto {
LOGGER_IMPL(magneto, "magneto")
}

int main(int argc, char** argv) {
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(test_ip_to_int, all) {
  /*
   * positive
   */
  uint32_t ip;
  bool ret = IOHelper::IpToInt("0.0.0.1", ip);
  ASSERT_TRUE(ret);
  ASSERT_EQ(1, ip);

  ret = IOHelper::IpToInt("1.0.0.0", ip);
  ASSERT_TRUE(ret);
  ASSERT_TRUE((1<<24)==ip);

  /*
   * negtive
   */
  ret = IOHelper::IpToInt("0.0.0.789", ip);
  ASSERT_TRUE(!ret);
}
