#include "gtest/gtest.h"
#include "../../../src/protocols/pool_protocols.h"

using namespace magneto;

namespace magneto {
LOGGER_IMPL(magneto, "magneto")
}

int main(int argc, char** argv) {
  srand(time(NULL));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(test_pool_protocols, all) {
  static const size_t kTimes = 10;

  ProtocolRead* protocol_read_ping[kTimes];
  ProtocolRead* protocol_read_rapid[kTimes];
  for (size_t i=0; i<kTimes; ++i) {
    protocol_read_ping[i] = PoolProtocols::GetRead(Protocol::kPing);
    ASSERT_TRUE(Protocol::kPing == protocol_read_ping[i]->GetCategory());

    protocol_read_rapid[i] = PoolProtocols::GetRead(Protocol::kRapid);
    ASSERT_TRUE(Protocol::kRapid == protocol_read_rapid[i]->GetCategory());
  }

  for (size_t i=0; i<kTimes; ++i) {
    PoolProtocols::FreeRead(protocol_read_ping[i]);
    PoolProtocols::FreeRead(protocol_read_rapid[i]);
  }
}

TEST(test_pool_protocols, pressure) {
  static const size_t kTimesTest=10000000;
  Timer timer;
  for (size_t i=0; i<kTimesTest; ++i) {
    PoolProtocols::FreeRead(PoolProtocols::GetRead(Protocol::kPing));
  }
  timer.Stop(true);
  std::cout << "qps " << kTimesTest*1000000/timer.TimeUs() << std::endl;
}
