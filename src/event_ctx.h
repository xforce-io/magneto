#pragma once

#include "public.h"
#include "protocols/pool_protocols.h"

namespace xforce { namespace magneto {

class Session;

struct EventCtx {
 public:
  enum Category {
    kListen,
    kReadReq,
    kSession,
    kMailboxNotify,
  };

 public:
  inline void BuildForListen(int fd, const ListenAddr& listen_addr);
  inline void BuildForReadReq(
      int fd, 
      int id_procedure,
      time_t fd_client_starttime_sec,
      ProtocolRead& protocol_read,
      time_t timeleft_ms);

  inline void BuildForSession(int fd, Session& session, size_t no_talk);
  inline void BuildForMailboxNotify(int fd);

  ~EventCtx() {}

 public:
  Category category;
  int fd;
  time_t timeleft_ms;
  time_t lasttime_ms;

  //data
  union {
    struct {
      const ListenAddr* listen_addr;
    } listen;

    struct {
      int id_procedure;
      time_t fd_client_starttime_sec;
      ProtocolRead* protocol_read;
    } read_req;

    struct {
      time_t fd_client_starttime_sec;
    } long_conn_wait;

    struct {
      Session* session;
      size_t no_talk;
    } session;
  } data;
};

void EventCtx::BuildForListen(int fd_arg, const ListenAddr& listen_addr) {
  category=kListen;
  fd=fd_arg;
  data.listen.listen_addr = &listen_addr;
}

void EventCtx::BuildForReadReq(
    int fd_arg, 
    int id_procedure, 
    time_t fd_client_starttime_sec,
    ProtocolRead& protocol_read,
    time_t timeleft_ms_arg) {
  category=kReadReq;
  fd=fd_arg;
  timeleft_ms=timeleft_ms_arg;
  lasttime_ms = Time::GetCurrentMsec(false);

  data.read_req.id_procedure = id_procedure;
  data.read_req.fd_client_starttime_sec = fd_client_starttime_sec;
  data.read_req.protocol_read = &protocol_read;
}

void EventCtx::BuildForSession(int fd_arg, Session& session, size_t no_talk) {
  category=kSession;
  fd=fd_arg;
  timeleft_ms = 0;
  lasttime_ms = Time::GetCurrentMsec(false);
  data.session.session = &session;
  data.session.no_talk = no_talk;
}

void EventCtx::BuildForMailboxNotify(int fd_arg) {
  category=kMailboxNotify;
  fd=fd_arg;
}

}}
