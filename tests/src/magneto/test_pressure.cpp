#include "gtest/gtest.h"
#include "../../../src/magneto.h"

namespace xforce { namespace magneto {
LOGGER_IMPL(xforce_logger, "magneto")
}}

magneto::Magneto server;

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

void NaiveHandler(void*) {
  size_t accu=0, succ=0, fail=0;
  magneto::ProtocolRead* response;
  magneto::Timer timer;

  timer.Start(true);
  for (;;) {
    int ret = server.SimpleTalk("downstream", magneto::Buf(magneto::Slice("a", 1), NULL), 100, response);
    magneto::ProtocolReadRedis* protocol_read_redis = SCAST<magneto::ProtocolReadRedis*>(response);
    if ( magneto::ErrorNo::kOk == ret && 'b' == protocol_read_redis->Data()[0] ) {
      ++succ;
    } else {
      ++fail;
    }
    server.FreeTalks();

    if ( (++accu % 10000) == 0 ) {
      timer.Stop(true);
      std::cout << "qps[" << 1000000*succ/timer.TimeUs() << "] fail[" << fail << "]" << std::endl;
    }
  }
}

TEST(test_magneto, all) {
  LOGGER_SYS_INIT("conf/log.conf")

  std::vector< std::pair<magneto::RoutineHandler, size_t> > routine_handlers;
  routine_handlers.push_back(std::make_pair(NaiveHandler, 30));
  bool end=false;
  bool ret = server.Init("data/magneto/test_pressure/conf/", NULL, &routine_handlers, NULL, end);
  ASSERT_TRUE(ret);
  sleep(1000);
  end=true;
}
