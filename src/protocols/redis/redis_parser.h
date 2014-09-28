#pragma once

#include "../public.h"

namespace magneto {
 
class RedisParser {
 public:
  static const char kSep = '\1';

 public:
  static int ParseReply(const std::string& reply, bool* is_pong, size_t* size_reply, std::string* out);
  static bool ParseCmd(const std::string& in, std::string* out);

 private: 
  static int ParseReplySeg_(const std::string& reply, size_t start, size_t* size_reply, std::string* out);
};

}
