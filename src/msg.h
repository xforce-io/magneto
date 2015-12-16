#pragma once

#include "public.h"
#include "protocols/protocol.h"

namespace xforce { namespace magneto {

class ConfServices;
class Talk;
class BizProcedure;
class Remote;
class ProtocolRead;

struct Msg {
 public:
  enum Category {
    kInvalid,
    kConfig,
    kReadReq,
    kNewReq,
    kSession,
    kDestruct,
    kNumMsgCategories,
  };

  static const size_t kMaxSizeMsg=10000;

 public:
  inline Msg();
  inline Msg(Msg::Category category_arg, uint32_t size_arg);
  inline void Copy(const Msg& msg);

 public:
  Category category;
  uint32_t size;
};

struct MsgConfig : public Msg {
 public:
  MsgConfig() : Msg(Msg::kConfig, sizeof(MsgConfig)) {} 
  void BuildForConfig() {}
};

struct MsgReadReq : public Msg {
 public: 
  MsgReadReq() : Msg(Msg::kReadReq, sizeof(MsgReadReq)) {}
  inline void BuildForReadReq(
      int fd_client, 
      const ListenAddr& listen_addr,
      time_t fd_client_starttime_sec=0);
 
 public: 
  int id_procedure;
  int fd_client;
  const ListenAddr* listen_addr;
  time_t fd_client_starttime_sec;
};

struct MsgNewReq : public Msg {
 public:
  MsgNewReq() : Msg(Msg::kNewReq, sizeof(MsgNewReq)) {} 
  inline void BuildForNewReq(
      int id_procedure, 
      int fd_client, 
      time_t fd_client_starttime_sec,
      ProtocolRead* protocol_read);
 
 public: 
  int id_procedure;
  int fd_client;
  time_t fd_client_starttime_sec;
  ProtocolRead* protocol_read;
};

struct MsgSession : public Msg {
 public: 
  enum Status {
    kOk,
    kTimeout,
    kBroken,
    kOther,
  };
 
 public:
  MsgSession() : Msg(Msg::kSession, sizeof(MsgSession)) {}
  void BuildForSession(
    ucontext_t& biz_ctx,
    BizProcedure& biz_procedure,
    std::vector<Talk>& talks,
    time_t timeo_ms,
    int error);

 public: 
  ucontext_t* biz_ctx;
  BizProcedure* biz_procedure;
  std::vector<Talk>* talks;
  time_t timeo_ms;
  int error;
};

struct MsgDestruct : public Msg {
 private: 
  static const size_t kSizeSmallCache=15;
 
 public:
  MsgDestruct() : Msg(Msg::kDestruct, sizeof(MsgDestruct)) {}
  void BuildForDestruct(
    int id_procedure,
    int fd_client,
    time_t fd_client_starttime_sec,
    ProtocolRead* protocol_read,
    const BizProcedure& biz_procedure);

 public:
  int id_procedure;
  int fd_client;
  time_t fd_client_starttime_sec;
  ProtocolRead* protocol_read;

  struct {
    std::pair<int, const Remote*> small_cache[kSizeSmallCache];
    size_t num;
  } small_cache;
  std::vector< std::pair<int, const Remote*> >* big_cache;
};

Msg::Msg() :
  category(Msg::kInvalid) {}

Msg::Msg(Msg::Category category_arg, uint32_t size_arg) :
  category(category_arg),
  size(size_arg) {}

void Msg::Copy(const Msg& msg) {
  switch (category) {
    case Msg::kConfig :
      SCAST<MsgConfig&>(*this) = SCAST<const MsgConfig&>(msg);
      break;
    case Msg::kReadReq :
      SCAST<MsgReadReq&>(*this) = SCAST<const MsgReadReq&>(msg);
      break;
    case Msg::kNewReq :
      SCAST<MsgNewReq&>(*this) = SCAST<const MsgNewReq&>(msg);
      break;
    case Msg::kSession :
      SCAST<MsgSession&>(*this) = SCAST<const MsgSession&>(msg);
      break;
    case Msg::kDestruct :
      SCAST<MsgDestruct&>(*this) = SCAST<const MsgDestruct&>(msg);
      break;
    default :
      XFC_BUG(true)
  }
}

void MsgReadReq::BuildForReadReq(
    int fd_client_arg, 
    const ListenAddr& listen_addr_arg,
    time_t fd_client_starttime_sec_arg) {
  static unsigned int seed=1;
  id_procedure = rand_r(&seed);
  fd_client=fd_client_arg;
  listen_addr = &listen_addr_arg;
  if (0==fd_client_starttime_sec_arg) {
    fd_client_starttime_sec = Time::GetCurrentSec(false);
  } else {
    fd_client_starttime_sec = fd_client_starttime_sec_arg;
  }
}

void MsgNewReq::BuildForNewReq(
    int id_procedure_arg,
    int fd_client_arg, 
    time_t fd_client_starttime_sec_arg,
    ProtocolRead* protocol_read_arg) {
  id_procedure=id_procedure_arg;
  fd_client=fd_client_arg;
  fd_client_starttime_sec=fd_client_starttime_sec_arg;
  protocol_read=protocol_read_arg;
}

}}
