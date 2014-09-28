#include "../redis_parser.h"

namespace magneto {

int RedisParser::ParseReply(const std::string& reply, bool* is_pong, size_t* size_reply, std::string* out) {
  if (0 == reply.length() || NULL==is_pong || NULL==size_reply || NULL==out) return -1;

  size_t idx_cr;
  char* endptr;
  *is_pong=false;

  idx_cr = reply.find("\r\n");
  if (std::string::npos==idx_cr) return 1;

  switch (reply[0]) {
    case '*' : {
      int64_t num_answer = strtol(reply.c_str()+1, &endptr, 10);
      if (LONG_MAX==num_answer || LONG_MIN==num_answer || endptr==reply.c_str()+1) return -1;

      if (num_answer<=0) {
        if (-1==num_answer) {
          (*out).append("0");
          *size_reply = idx_cr+2;
          return 0;
        } else {
          return -1;
        }
      }

      static const size_t kMaxLenInt64 = 30;
      char int_buf[kMaxLenInt64+1];
      sprintf(int_buf, "%lu", num_answer);
      (*out).append(int_buf).append(1, kSep);

      size_t idx_dollar=0;
      size_t size_reply_seg=0;
      for (int i=0; i<num_answer; ++i) {
        idx_dollar = reply.find('$', idx_dollar);
        if (std::string::npos==idx_dollar) return 1;

        int ret = ParseReplySeg_(reply, idx_dollar, &size_reply_seg, out);
        if (0!=ret) return ret;

        if (num_answer-1 != i) (*out).append(1, kSep);
        ++idx_dollar;
      }
      *size_reply = idx_dollar+size_reply_seg-1;
      return 0;
    }
    case '$' : {
      int64_t len_answer = strtol(reply.c_str()+1, &endptr, 10);
      if (LONG_MAX==len_answer || LONG_MIN==len_answer || endptr==reply.c_str()+1) return -1;

      if (-1!=len_answer) {
        size_t idx_second_cr = reply.find("\r\n", idx_cr+1);
        if (std::string::npos==idx_second_cr ) {
          return 1;
        } else if(static_cast<int>(idx_second_cr-idx_cr) != 2+len_answer) {
          return -1;
        }

        *size_reply = idx_second_cr+2;
        (*out).append("1").append(1, kSep).append(reply.substr(idx_cr+2, len_answer));
      } else {
        *size_reply = idx_cr+2;
        (*out).append("0");
      }
      return 0;
    }
    case ':' : {
      *size_reply = idx_cr+2;
      (*out).append("1").append(1, kSep).append(reply.substr(1, idx_cr-1));
      return 0;
    }
    case '+' : {
      *size_reply = idx_cr+2;
      if (reply.length() >= 5 && std::string::npos != reply.find("PONG")) *is_pong=true;
      (*out).append("0");
      return 0;
    }
    case '-' : {
      *size_reply = idx_cr+2;
      (*out).append("null");
      return 0;
    }
    default :
      return -1;
  }
}

bool RedisParser::ParseCmd(const std::string& in, std::string* out) {
  if (0==in.length() || NULL==out) return false;

  std::string result("\r\n");
  size_t follower=0;
  size_t leader;
  uint32_t num_segs=0;

  static const size_t kMaxLenInt64 = 30;
  char int_buf[kMaxLenInt64+1];
  do {
    leader = in.find(kSep, follower);
    if (leader==follower) return false;

    uint32_t len_seg = (std::string::npos!=leader ? leader-follower : in.length()-follower);
    if (0==len_seg) break;

    ++num_segs;

    sprintf(int_buf, "%u", len_seg);
    result.append("$").append(int_buf).append("\r\n").append(in, follower, len_seg).append("\r\n");

    follower=leader+1;
  } while(leader!=std::string::npos);

  sprintf(int_buf, "%u", num_segs);
  (*out).assign("*").append(int_buf).append(result);
  return true;
}

int RedisParser::ParseReplySeg_(
    const std::string& reply,
    size_t start, 
    size_t* size_reply_seg, 
    std::string* out) {
  size_t idx_cr = reply.find("\r\n", start);
  if (std::string::npos==idx_cr) return 1;

  char* endptr;
  int64_t len_answer = strtol(reply.c_str()+start+1, &endptr, 10);
  if (-1==len_answer) {
    *size_reply_seg = idx_cr+2-start;
    (*out).append("null");
    return 0;
  } else if (LONG_MAX==len_answer
    || LONG_MIN==len_answer
    || endptr==reply.c_str()+3
    || len_answer<=0) {
    return -1;
  }

  size_t idx_second_cr = reply.find("\r\n", idx_cr+1);
  if (std::string::npos==idx_second_cr) return 1;

  if (static_cast<int>(idx_second_cr-idx_cr-2) != len_answer) return -1;

  (*out).append(reply, idx_cr+2, len_answer);
  *size_reply_seg = idx_second_cr+2-start;
  return 0;
}

}
