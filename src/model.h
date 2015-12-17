#pragma once 

#include <sys/socket.h>
#include <netinet/in.h>
#include "public.h"
#include "protocols/pool_protocols.h"
#include "protocols/protocol.h"

namespace xforce { namespace magneto {

class ProtocolWrite;
class ProtocolRead;

struct Remote {
 public: 
  struct HashRemote {
    size_t operator()(const Remote& remote) const {
      return std::tr1::hash<size_t>()( ((size_t)(remote.addr.addr.sin_port) << 32) + 
        remote.addr.addr.sin_addr.s_addr );
    }
  };

  struct EqRemote {
    bool operator()(const Remote& left, const Remote& right) const {
      return left.addr.addr.sin_addr.s_addr == right.addr.addr.sin_addr.s_addr
        && left.addr.addr.sin_port == right.addr.addr.sin_port;
    }
  };

  Remote& operator=(const Remote& other) {
    if (unlikely(this == &other)) return *this;

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
  Addr addr;
  size_t weight;
  size_t long_conns;
  time_t ctimeo_ms;
  time_t rtimeo_ms;
  time_t wtimeo_ms;
};

struct ServiceStrategy {
 public: 
  enum Type {
    kNormal,
    kWeight,
    kInvalid,
  };

 public: 
  static const Type kDefaultStrategy = kWeight;

 public: 
  inline static Type GetServiceStrategy(const std::string& service_strategy);
};

struct Service {
 public:
  typedef std::vector<Remote> Remotes;

 public: 
  size_t no;
  std::string name;
  Protocol::Category protocol_category;
  ServiceStrategy::Type service_strategy;
  Remotes remotes;
  size_t weight_all;
  size_t retry;
};

typedef std::vector<const Service*> ServicesSet;

struct TypeSession {
  enum Type {
    kServiceSet,
    kService,
    kRemote,
    kInvalid,
  };
};

struct Talk {
 public: 
  enum Category {
    kWriteOnly,
    kReadOnly,
    kWriteAndRead,
  };

  enum Status {
    kStart,
    kConn,
    kRead,
    kWrite,
    kClose,
    kEnd,
  };

  size_t no_talk;
  Category category;
  const Service* service;
  const Buf* buf;
  ProtocolWrite* protocol_write;
  ProtocolRead* protocol_read;
  time_t starttime_ms;
  time_t endtime_ms;
  Status status;
  int error;
  std::vector<Remote> fail_remotes;
  size_t retry;

  int fd;
  const Remote* remote;

 public:
  void Assign(
      size_t no_talk,
      Category category, 
      const Service* service, 
      Protocol::Category protocol_category,
      const Buf* buf,
      time_t timeo_ms, 
      int fd, 
      const Remote* remote);
};

ServiceStrategy::Type ServiceStrategy::GetServiceStrategy(const std::string& service_strategy) {
  if (service_strategy == "normal") {
    return kNormal;
  } else if (service_strategy == "weight") {
    return kWeight;
  } else {
    return kInvalid;
  }
}

}}
