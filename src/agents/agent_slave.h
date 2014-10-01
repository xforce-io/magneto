#pragma once 

#include "public.h"
#include "../model.h"
#include "../msg.h"
#include "mailbox_notifier.h"

namespace magneto {

class Confs;
class Schedulers;
class ConnsMgr;
class Session;
class EventCtx;

class AgentSlave {
 public: 
  typedef AgentSlave Self;
  typedef MPSCFixedPipe<bool, MailboxNotifier> Mailbox;
  typedef EventsDriver<EventCtx, SpinLock> Driver;

 private: 
  static const size_t kMaxNumMailsProcessedOnce=100000;
  static const size_t kBacklog=1024;
  static const size_t kThreholdSenderNotify=3;

 public:
  AgentSlave();
  bool Init(const Confs& confs, Schedulers& schedulers);
  bool Start();

  inline bool SendMsg(Msg& msg);
  int GetNumClientsKeepalive() const { return num_clients_keepalive_; }

  void Stop();
  virtual ~AgentSlave();

 private:
  static void* Run_(void* args);
  void Process_();
  bool CheckEvents_();
  bool CleanTimeouts_();
  bool CheckMailbox_();

  /*
   * @return: 
   *    true  : finish
   *    false : more needs to be handled
   */
  bool CheckSession_(Driver::Event event, EventCtx& event_ctx);

  /*
   * @return: whether need more events
   */
  bool HandleSessionEventConn_(Talk& talk, EventCtx& event_ctx);
  bool HandleSessionEventRead_(Talk& talk, EventCtx& event_ctx);
  bool HandleSessionEventWrite_(Talk& talk, EventCtx& event_ctx);

  void HandleMsgDestruct_(MsgDestruct& msg_destruct);

  /*
   * @return:
   *    ==0 : ok
   *    > 0 : need more
   *    < 0 : error happen
   */
  int HandleReadReq_(EventCtx& event_ctx);

  inline void SendBackMsg_(Msg& msg);
  inline Msg* ReceiveMsg_();
  void MsgConsumed_() { mailbox_.MsgConsumed(); }
  inline void CloseFdClient_(int& fd);
  inline void CloseFd_(int& fd);

  /*
   * @return : true[not timeout] false[timeout] 
   */
  inline bool CheckEventCtxTimeout_(EventCtx& event_ctx, time_t timeleft_ms=0);

 private:
  const Confs* confs_;
  Schedulers* schedulers_;

  Driver events_driver_;
  ConnsMgr* conns_mgr_;
  MailboxNotifier* mailbox_notifier_;
  Mailbox mailbox_;
  PoolObjs<EventCtx>* pool_event_ctxs_;
  PoolObjsInit<Session>* pool_sessions_;
  pthread_t tid_agent_slave_;

  int num_clients_keepalive_;

  DelayCloseFds delay_close_fds_;

  MsgNewReq tmp_msg_new_req_;
  MsgSession tmp_msg_session_;
  MsgDestruct tmp_msg_destruct_;

  bool end_;
};

}

#include "../schedulers/scheduler.h"
#include "../event_ctx.h"

namespace magneto {

bool AgentSlave::SendMsg(Msg& msg) {
  return mailbox_.SendMsg(true, RCAST<const char*>(&msg), msg.size);
}

void AgentSlave::SendBackMsg_(Msg& msg) {
  MAG_BUG(Msg::kSession != msg.category)

  MsgSession& msg_session = SCAST<MsgSession&>(msg);
  Scheduler& scheduler = *(msg_session.biz_procedure->GetScheduler());

  bool ret = scheduler.SendMsg(msg);
  if (unlikely(!ret)) {
    size_t times=0;
    do {
      ret = scheduler.SendMsg(msg);
      ++times;

      static const size_t kThreholdWarn=10000;
      if (0 == times%kThreholdWarn) {
        WARN("agent_slave_wait_to_send_back_msg times[" << times << "]");
        usleep(1000);
      }
    } while (!ret);
  }
}

Msg* AgentSlave::ReceiveMsg_() { 
  Mailbox::Msg* msg_mailbox = mailbox_.ReceiveMsg(); 
  return NULL!=msg_mailbox ? RCAST<Msg*>(msg_mailbox->content) : NULL;
}

void AgentSlave::CloseFdClient_(int& fd) {
  TRACE("num_clients_keepalive_dec[" 
      << fd 
      << "] num_clients_keepalive_[" 
      << num_clients_keepalive_
      << "]");
  --num_clients_keepalive_;
  CloseFd_(fd);
}

void AgentSlave::CloseFd_(int& fd) {
  delay_close_fds_.Close(fd);
  fd=-1;
}

bool AgentSlave::CheckEventCtxTimeout_(EventCtx& event_ctx, time_t timeleft_ms) {
  time_t cur_time_ms = Time::GetCurrentMsec(false);
  time_t time_consumed_ms = cur_time_ms - event_ctx.lasttime_ms;
  if (time_consumed_ms >= event_ctx.timeleft_ms) {
    WARN("cur_time_ms[" 
        << cur_time_ms
        << "] event_ctx.lasttime_ms[" 
        << event_ctx.lasttime_ms
        << "] time_consumed_ms[" 
        <<  time_consumed_ms
        << "] timeleft_ms[" 
        << event_ctx.timeleft_ms
        << "]");
    return false;
  }

  if (timeleft_ms) {
    event_ctx.timeleft_ms = timeleft_ms;
  } else {
    event_ctx.timeleft_ms -= time_consumed_ms;
  }
  event_ctx.lasttime_ms = cur_time_ms;
  return true;
}

}
