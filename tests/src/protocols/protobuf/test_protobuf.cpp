#include "gtest/gtest.h"

#include "../../../../src/protocols/protobuf/protocol_protobuf.h"
#include "sample.pb.h"

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

TEST(protobuf, encode_and_decode) {
  ProtocolWriteProtobuf protocol_write;
  ::Test::Msg msg;
  msg.set_userid(1);
  msg.set_name("2");
  msg.add_creattime(3);

  Buf buf;
  buf.first.Set(RCAST<char*>(&msg), 0);
  buf.second = RCAST<const void*>("Test.Msg");
  protocol_write.Reset(buf);
  ASSERT_TRUE(protocol_write.Encode());

  ProtocolReadProtobuf protocol_read;
  protocol_read.buffer_.Set(protocol_write.out_.data(), protocol_write.out_.size());
  ASSERT_TRUE(protocol_read.Decode());
  const ::Test::Msg* msg_out = RCAST<const ::Test::Msg*>(protocol_read.GetMsg());
  ASSERT_TRUE(msg_out->userid() == 1);
  ASSERT_TRUE(msg_out->name() == "2");
  ASSERT_TRUE(msg_out->creattime(0) == 3);
  ASSERT_TRUE(protocol_read.GetMsgName() == "Test.Msg");
}
