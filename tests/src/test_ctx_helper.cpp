#include "gtest/gtest.h"
#include "../../src/ctx_helper.h"

using namespace magneto;

namespace xforce { namespace magneto {
LOGGER_IMPL(xforce_logger, "magneto")
}}

int main(int argc, char** argv) {
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(test_all, all) {
  int a=1;
  int hign, low;
  CtxHelper::PtrToInts<int>(&a, hign, low);
  int* ptr_a = CtxHelper::IntsToPtr<int>(hign, low);
  ASSERT_TRUE(1==*ptr_a);
}
