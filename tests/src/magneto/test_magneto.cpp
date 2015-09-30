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

void PingHandler(const magneto::ProtocolRead& protocol_read, void*) {
  const magneto::ProtocolReadPing& protocol_read_ping = SCAST<const magneto::ProtocolReadPing&>(protocol_read);
  if ( 1 != protocol_read_ping.Size() ) {
    WARN_LOG(magneto::xforce_logger, "ret[" << protocol_read_ping.Size() << "]");
  }
/*
  magneto::ProtocolRead* response;
  int ret = server.SimpleTalk("redis", std::make_pair("GET""\1""1", 6), 100, response);
  magneto::ProtocolReadRedis* protocol_read_redis = SCAST<magneto::ProtocolReadRedis*>(response);
  if (magneto::ErrorNo::kOk != ret) {
    WARN("fail_simple_talk_to_service0 ret[" << ret << "]");
  }
*/
  server.WriteBack(magneto::Buf(magneto::Slice("b", 1), NULL), 100);
}

TEST(test_magneto, all) {
  LOGGER_SYS_INIT("conf/log.conf")

  bool end=false;
  bool ret = server.Init("data/magneto/test_magneto/conf/", &PingHandler, NULL, NULL, end);
  ASSERT_TRUE(ret);
  sleep(1000);
  end=true;
}
