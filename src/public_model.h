#pragma once

#include "../deps/public-cpp/include/public-cpp/slice.hpp"
#include "handlers.h"

namespace xforce { namespace magneto {

typedef std::pair<Slice, const void*> Buf;
typedef std::vector<const Buf*> Bufs;
typedef std::pair<RoutineHandler, size_t> RoutineItem;
typedef std::vector<RoutineItem> RoutineItems;
typedef std::pair<int, ProtocolRead*> Response;
typedef std::vector<Response> Responses;
typedef std::vector<int> Errors;

struct Addr {
 public: 
  sockaddr_in addr;

 public:
  inline bool Assign(const std::string& addr);
};

struct Remote {
 public: 
  Remote& operator=(const Remote& other) {
    if (unlikely(this == &other)) return *this;

    name = other.name;
    addr.addr.sin_addr.s_addr = other.addr.addr.sin_addr.s_addr;
    addr.addr.sin_port = other.addr.addr.sin_port;
    weight = other.weight;
    long_conns = other.long_conns;
    ctimeo_ms = other.ctimeo_ms;
    rtimeo_ms = other.rtimeo_ms;
    wtimeo_ms = other.wtimeo_ms;
    return *this;
  }

 public:
  std::string name;
  Addr addr;
  size_t weight;
  size_t long_conns;
  time_t ctimeo_ms;
  time_t rtimeo_ms;
  time_t wtimeo_ms;
};

}}
