#include "../protocol_redis.h"
#include "../redis_parser.h"

namespace xforce { namespace magneto {

bool ProtocolWriteRedis::Encode() {
  return RedisParser::ParseCmd(query_, &out_);
}

int ProtocolWriteRedis::Write(int fd) {
  int ret = IOHelper::WriteNonBlock(fd, out_.data() + out_pos_, out_.size() - out_pos_);
  if (ret>0) {
    out_pos_+=ret;
    return out_.size() - out_pos_;
  } else {
    return 0==ret ? kEnd : ret;
  }
}

ProtocolReadRedis::ProtocolReadRedis() {
  XFC_NEW(tmpbuf_, char [kTmpBufSize+1])
}

int ProtocolReadRedis::Read(int fd) { 
  while (true) {
    size_t tmppos_=0;
    int ret = IOHelper::ReadNonBlock(fd, tmpbuf_, kTmpBufSize);
    if (ret>=0) {
      tmppos_+=ret;
      tmpbuf_[tmppos_] = '\0';
      in_.append(tmpbuf_);
      if (ret<kTmpBufSize) {
        break;
      }
    } else {
      return -1;
    }
  }

  bool is_pong;
  size_t size_reply;
  return RedisParser::ParseReply(in_, &is_pong, &size_reply, &reply_); 
}

ProtocolReadRedis::~ProtocolReadRedis() {
  XFC_DELETE_ARRAY(tmpbuf_)
}

}}
