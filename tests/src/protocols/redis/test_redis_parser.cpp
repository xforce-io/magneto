#include "gtest/gtest.h"
#include "../../../../src/protocols/redis/redis_parser.h"

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

class TestRedisParser : public ::testing::Test {
 protected:
  virtual ~TestRedisParser() {};
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(TestRedisParser, ParseReplySeg) {
  //positive
  std::string reply("$5\r\nworld\r\n");
  size_t size_reply_seg;
  std::string result;
  int ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "world");
  ASSERT_EQ(size_reply_seg, 11);

  reply = std::string("$-1\r\n");
  result.clear();
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "__null");
  ASSERT_EQ(size_reply_seg, 5);

  reply = std::string("$-1\r");
  result.clear();
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, 1);

  reply = std::string("$-1");
  result.clear();
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, 1);

  reply = std::string("$1\r");
  result.clear();
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, 1);

  //passive
  reply = std::string("$5\r\nworld\r");
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, 1);

  reply = std::string("$5\r\nwold\r\n");
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, -1);

  reply = std::string("$\r\nworld\r\n");
  ret = RedisParser::ParseReplySeg_(reply, 0, &size_reply_seg, &result);
  ASSERT_EQ(ret, -1);
}

TEST_F(TestRedisParser, ParseReply) {
  std::string reply(":1000\r\n");
  size_t size_reply;
  std::string result;
  bool is_pong;

  /*
   * ":" reply
   */
  int ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "1""\1""1000");
  ASSERT_EQ(size_reply, 7);

  reply = std::string(":1000");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);

  /*
   * "+" reply
   */
  reply = std::string("+OK\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "0");
  ASSERT_EQ(size_reply, 5);
  ASSERT_EQ(is_pong, false);

  reply = std::string("+OK");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);
  ASSERT_EQ(is_pong, false);

  reply = std::string("+PONG\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "0");
  ASSERT_EQ(size_reply, 7);
  ASSERT_EQ(is_pong, true);

  /*
   * "-" reply
   */
  reply = std::string("-Error\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "__null");
  ASSERT_EQ(size_reply, 8);

  reply = std::string("-");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);

  /*
   * "$" reply
   */
  //positive
  reply = std::string("$5\r\nvalue\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "1""\1""value");
  ASSERT_EQ(size_reply, 11);

  //passive
  reply = std::string("$-1\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "0");
  ASSERT_EQ(size_reply, 5);

  reply = std::string("$-1");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);

  reply = std::string("$1\r\nvalue\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, -1);

  reply = std::string("$1\r\nvalue");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);

  /*
   * '*' reply
   */
  //positive
  reply = std::string("*2\r\n$3\r\nGET\r\n$2\r\nas\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "2""\1""GET""\1""as");
  ASSERT_EQ(size_reply, 21);

  reply = std::string("*2\r\n$-1\r\n$-1\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "2""\1""__null""\1""__null");
  ASSERT_EQ(size_reply, 14);

  reply = std::string("*-1\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(result, "0");
  ASSERT_EQ(size_reply, 5);

  //passive
  ret = RedisParser::ParseReply(reply, NULL, NULL, NULL);
  ASSERT_EQ(ret, -1);
  
  reply = std::string("*2");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);

  reply = std::string("*2\r\n$3\r\nGET\r\n$2\r\na\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, -1);

  reply = std::string("*2\r\n$3\r\nGET\r\n");
  result.clear();
  ret = RedisParser::ParseReply(reply, &is_pong, &size_reply, &result);
  ASSERT_EQ(ret, 1);
}

TEST_F(TestRedisParser, ParseCmd) {
  std::string request("MGET""\1""1""\1""22""\1""3");
  std::string result;

  bool ret = RedisParser::ParseCmd(request, &result);
  ASSERT_EQ(ret, true);
  ASSERT_EQ(result, "*4\r\n$4\r\nMGET\r\n$1\r\n1\r\n$2\r\n22\r\n$1\r\n3\r\n");

  request = std::string("MGET""\1");
  ret = RedisParser::ParseCmd(request, &result);
  ASSERT_EQ(ret, true);
  ASSERT_EQ(result, "*1\r\n$4\r\nMGET\r\n");

  request = std::string("\1""MGET""\1""1""\1""22""\1""3");
  ret = RedisParser::ParseCmd(request, &result);
  ASSERT_EQ(ret, false);

  request = std::string("MGET""\1""1""\1""22""\1""\1""3");
  ret = RedisParser::ParseCmd(request, &result);
  ASSERT_EQ(ret, false);
}
