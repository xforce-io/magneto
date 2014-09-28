#include "../agent_slave.h"

#include "../public.h"
#include "../../event_ctx.h"
#include "../../conns_mgr/conns_mgr.h"
#include "../../protocols/protocol.h"
#include "../../confs/confs.h"
#include "../../session.h"
#include "../../schedulers/schedulers.h"

namespace magneto {

AgentSlave::AgentSlave() :
  conns_mgr_(NULL),
  pool_event_ctxs_(NULL),
  pool_sessions_(NULL),
  tid_agent_slave_(0),
  num_clients_keepalive_(0),
  end_(false) {}

bool AgentSlave::Init(const Confs& confs, Schedulers& schedulers) {
  confs_ = &confs;
  schedulers_ = &schedulers;
  MAG_NEW(conns_mgr_, ConnsMgr(confs.GetConfNormal().GetLongConnKeepaliveSec()))
  conns_mgr_->ConfigRemotes(confs_->GetConfServices()->GetRemotes());
  MAG_NEW(pool_event_ctxs_, PoolObjs<EventCtx>)
  MAG_NEW(mailbox_notifier_, MailboxNotifier(events_driver_))
  bool ret = mailbox_.Init(
      confs.GetConfNormal().GetSizeSchedulerMailbox(), 
      Msg::kMaxSizeMsg, 
      kThreholdSenderNotify, 
      mailbox_notifier_);
  if (true!=ret) {
    FATAL("fail_init_mailbox");
    return false;
  }

  MAG_NEW(
      pool_sessions_, 
      PoolObjsInit<Session>(Session(confs, events_driver_, *conns_mgr_, *pool_event_ctxs_)))
  return true;
}

bool AgentSlave::Start() {
  int ret = pthread_create(&tid_agent_slave_, NULL, Run_, this);
  if (0!=ret) {
    FATAL("fail_spawn_agent_slave_thread");
    return false;
  }
  return true;
}

void AgentSlave::Stop() {
  end_=true;
  if (0!=tid_agent_slave_) {
    pthread_join(tid_agent_slave_, NULL);
    tid_agent_slave_=0;
  }

  std::vector<EventCtx*>* event_ctxs = events_driver_.ClearAll();
  for (size_t i=0; i < event_ctxs->size(); ++i) {
    EventCtx& event_ctx = *((*event_ctxs)[i]);
    if (EventCtx::kReadReq == event_ctx.category && NULL != event_ctx.data.read_req.protocol_read) {
      PoolProtocols::FreeRead(event_ctx.data.read_req.protocol_read);
    }
    pool_event_ctxs_->Free(&event_ctx);
  }
}

AgentSlave::~AgentSlave() {
  Stop();
  MAG_DELETE(pool_sessions_)
  MAG_DELETE(pool_event_ctxs_)
  MAG_DELETE(mailbox_notifier_)
  MAG_DELETE(conns_mgr_)
}

void* AgentSlave::Run_(void* args) {
  prctl(PR_SET_NAME, "agent_slave");

  Self& self = *(RCAST<Self*>(args));
  while (!(self.end_)) {
    self.Process_();
  }
  self.Process_();
  return NULL;
}

void AgentSlave::Process_() {
  bool ret_check_events;  
  bool ret_check_timeouts;
  bool ret_check_mailbox;
  do {
    ret_check_events = CheckEvents_();
    ret_check_timeouts = CleanTimeouts_();
    ret_check_mailbox = CheckMailbox_();
  } while (ret_check_events 
      || ret_check_timeouts 
      || ret_check_mailbox);
}

bool AgentSlave::CheckEvents_() {
  int num_events_ready = events_driver_.Wait(!(confs_->GetConfNormal().GetLowLatency()));
  if (num_events_ready>0) {
    int ret;
    Driver::Event event = Driver::kErr;
    EventCtx* event_ctx=NULL;
    for (int i=0; i<num_events_ready; ++i) {
      events_driver_.CheckReadyEvent(i, event, event_ctx);
      switch (event_ctx->category) {
        case EventCtx::kSession : {
          ret = CheckSession_(event, *event_ctx);
          TRACE("CheckEvents_kSession[" 
              << Time::GetCurrentUsec(true)
              << "] event[" 
              << event
              << "] fd[" 
              << event_ctx->fd
              << "] fd_client[" 
              << event_ctx->data.session.session->GetBizProcedure()->GetFdClient()
              << "] ret[" 
              << ret
              << "]");
          if (true==ret) {
            Session& session = *(event_ctx->data.session.session);
            tmp_msg_session_.BuildForSession(
                *(session.GetBizCtx()), 
                *(session.GetBizProcedure()),
                *(session.GetTalks()), 
                0,
                (!session.HasFailure() ? ErrorNo::kOk : ErrorNo::kPartial));
            SendBackMsg_(tmp_msg_session_);
            pool_sessions_->Free(&session);
            pool_event_ctxs_->Free(event_ctx);
          }
          break;
        }
        case EventCtx::kReadReq : {
          if (Driver::kErr != event) {
            ret = HandleReadReq_(*event_ctx);
            TRACE("CheckEvents_kReadReq fd[" 
                << event_ctx->fd 
                << "] ret[" 
                << ret 
                << "] time[" 
                << Time::GetCurrentUsec(true) 
                << "]");
            if (0==ret) {
              tmp_msg_new_req_.BuildForNewReq(
                  event_ctx->data.read_req.id_procedure,
                  event_ctx->fd,
                  event_ctx->data.read_req.fd_client_starttime_sec, 
                  event_ctx->data.read_req.protocol_read);
              ret = schedulers_->SendToOneWorker(tmp_msg_new_req_);
              if (true==ret) {
                event_ctx->data.read_req.protocol_read = NULL;
              } else {
                WARN("fail_send_new_req_msg_to_schedulers");
                MAG_FAIL_HANDLE(true)
              }
            } else if (ret>0) {
              break;
            } else {
              MAG_FAIL_HANDLE(true)
            }
          } else {
            events_driver_.RegEventDel(event_ctx->fd);
            MAG_FAIL_HANDLE(true)
            DEBUG("invalid_read_req_event fd[" << event_ctx->fd << "]");
          }
          pool_event_ctxs_->Free(event_ctx);
          break;

          ERROR_HANDLE:
          CloseFdClient_(event_ctx->fd);
          PoolProtocols::FreeRead(event_ctx->data.read_req.protocol_read);
          pool_event_ctxs_->Free(event_ctx);
          break;
        }
        case EventCtx::kMailboxNotify : {
          MailboxNotifier::ClearReadBuf(event_ctx->fd);
          break;
        }
        default : {
          MAG_BUG(true)
          break;
        }
      }
    }
    return true;
  } else {
    if (num_events_ready<0) {
      FATAL("fail_wait_for_events reason[" << strerror(errno) << "]");
    }
    return false;
  }
}

bool AgentSlave::CleanTimeouts_() {
  const std::vector<EventCtx*>& timeouts = *(events_driver_.RemoveTimeouts());
  for (size_t i=0; i < timeouts.size(); ++i) {
    EventCtx* event_ctx = timeouts[i];
    MAG_BUG(NULL==event_ctx)

    switch (event_ctx->category) {
      case EventCtx::kSession : {
        Session& session = *(event_ctx->data.session.session);

        Talk& talk = (*session.GetTalks())[event_ctx->data.session.no_talk];
        CloseFd_(talk.fd);

        bool ret = session.FinishTalk(false, talk);
        if (ret) {
          tmp_msg_session_.BuildForSession(
              *(event_ctx->data.session.session->GetBizCtx()), 
              *(event_ctx->data.session.session->GetBizProcedure()),
              *(event_ctx->data.session.session->GetTalks()),
              0,
              ErrorNo::kPartial);
          SendBackMsg_(tmp_msg_session_);

          pool_sessions_->Free(event_ctx->data.session.session);
        }
        break;
      }
      case EventCtx::kReadReq : {
        CloseFdClient_(event_ctx->fd);
        if (NULL != event_ctx->data.read_req.protocol_read) {
          PoolProtocols::FreeRead(event_ctx->data.read_req.protocol_read);
        }
        break;
      }
      default :
        MAG_BUG(true)
    }
    pool_event_ctxs_->Free(event_ctx);
  }
  return 0 != timeouts.size();
}

bool AgentSlave::CheckMailbox_() {
  size_t num_mails=0;
  while (num_mails<kMaxNumMailsProcessedOnce) {
    Msg* msg = ReceiveMsg_();
    if (unlikely(NULL==msg)) break;

    switch (msg->category) {
      case Msg::kConfig : {
        conns_mgr_->ConfigRemotes(confs_->GetConfServices()->GetRemotes());
        break;
      }
      case Msg::kSession : {
        MsgSession& msg_session = *SCAST<MsgSession*>(msg);
        TRACE("CheckMailbox_kSession["
            << Time::GetCurrentUsec(true)
            << "] fd_client["
            << msg_session.biz_procedure->GetFdClient()
            << "]");

        Session* session = pool_sessions_->Get();    
        bool ret = session->Reset(
            *(msg_session.biz_ctx), 
            *(msg_session.biz_procedure), 
            *(msg_session.talks));
        if (unlikely(!ret)) {
          msg_session.error = MsgSession::kBroken;
          SendBackMsg_(*msg);
          WARN("fail_reset_session");
        }
        break;
      }
      case Msg::kReadReq : {
        ++num_clients_keepalive_;

        MsgReadReq& msg_read_req = *SCAST<MsgReadReq*>(msg);
        TRACE("CheckMailbox_kReadReq["
          << Time::GetCurrentUsec(true) 
          << "] fd_client["
          << msg_read_req.fd_client
          << "]");

        ProtocolRead* protocol_read = PoolProtocols::GetRead(msg_read_req.listen_addr->category);
        protocol_read->Reset(msg_read_req.listen_addr);

        EventCtx* event_ctx = pool_event_ctxs_->Get();
        event_ctx->BuildForReadReq(
            msg_read_req.fd_client, 
            msg_read_req.id_procedure,
            msg_read_req.fd_client_starttime_sec,
            *protocol_read,
            confs_->GetConfNormal().GetRtimeoMs());

        bool ret = events_driver_.RegEvent(
            msg_read_req.fd_client,
            Driver::kAddEvent,
            Driver::kIn,
            event_ctx,
            event_ctx->timeleft_ms);
        if (!ret) {
          CloseFdClient_(msg_read_req.fd_client);
          PoolProtocols::FreeRead(protocol_read);
          pool_event_ctxs_->Free(event_ctx);
          WARN("fail_reg_event");
        }
        break;
      }
      case Msg::kDestruct : {
        MsgDestruct& msg_destruct = *SCAST<MsgDestruct*>(msg);
        TRACE("CheckMailbox_kDestruct[" << Time::GetCurrentUsec(true) << "]");
        HandleMsgDestruct_(msg_destruct);
        break;
      }
      default:
        MAG_BUG(true);
        break;
    }

    MsgConsumed_();
    ++num_mails;
  }
  return 0!=num_mails;
}

bool AgentSlave::CheckSession_(Driver::Event event, EventCtx& event_ctx) {
  Session& session = *(event_ctx.data.session.session);
  Talk& talk = session.GetTalk(event_ctx.data.session.no_talk);
  if (Driver::kIn == event || Driver::kOut == event) {
    switch (talk.status) {
      case Talk::kConn : {
        if (HandleSessionEventConn_(talk, event_ctx)) {
          return false;
        }
        break;
      }
      case Talk::kRead : {
        if (HandleSessionEventRead_(talk, event_ctx)) {
          return false;
        }
        break;
      }
      case Talk::kWrite : {
        if (HandleSessionEventWrite_(talk, event_ctx)) {
          return false;
        }
        break;
      }
      default : {
        MAG_BUG(true)
      }
    }
  } else {
    events_driver_.RegEventDel(talk.fd);
    CloseFd_(talk.fd);
    talk.error = (Driver::kErr == event ? ErrorNo::kBroken : ErrorNo::kTimeout);
  }

  if (ErrorNo::kOk != talk.error) {
    DEBUG("talk_err[" 
        << talk.error
        << "] fd[" 
        << talk.fd
        << "] fd_client[" 
        << session.GetBizProcedure()->GetFdClient()
        << "]");
  }
  return session.FinishTalk(ErrorNo::kOk == talk.error, talk);
}

bool AgentSlave::HandleSessionEventConn_(Talk& talk, EventCtx& event_ctx) {
  bool ret;
  Driver::Event event;

  talk.status = (Talk::kReadOnly == talk.category ? Talk::kRead : Talk::kWrite);
  ret = CheckEventCtxTimeout_(
      event_ctx, 
      std::min(
          talk.endtime_ms - Time::GetCurrentMsec(false),
          (Talk::kReadOnly != talk.category ? 
            talk.remote->wtimeo_ms :
            talk.remote->rtimeo_ms)));
  MAG_FAIL_HANDLE_AND_SET(!ret, talk.error = ErrorNo::kTimeout)

  event = (Talk::kReadOnly != talk.category ? Driver::kOut: Driver::kIn);
  ret = events_driver_.RegEvent(
      talk.fd, 
      Driver::kModEvent, 
      event, 
      &event_ctx, 
      event_ctx.timeleft_ms);
  MAG_FAIL_HANDLE_AND_SET(!ret, talk.error = ErrorNo::kOther)
  return true;

  ERROR_HANDLE:
  events_driver_.RegEventDel(talk.fd);
  if (ErrorNo::kOk != talk.error) {
    WARN("fail_handle_session_event_conn");
    CloseFd_(talk.fd);
  }
  return false;
}

bool AgentSlave::HandleSessionEventRead_(Talk& talk, EventCtx& event_ctx) {
  ProtocolRead& protocol_read = *(talk.protocol_read);
  int ret = protocol_read.Read(talk.fd);
  if (0==ret) { 
    ret = protocol_read.Decode();
    MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kDecode)

    MAG_FAIL_HANDLE_AND_SET(true, talk.error = ErrorNo::kOk)
  } else if (ret>0) {
    ret = CheckEventCtxTimeout_(event_ctx);
    MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kTimeout)

    ret = events_driver_.RegEvent(
        talk.fd, 
        Driver::kModEvent, 
        Driver::kIn, 
        &event_ctx, 
        event_ctx.timeleft_ms);
    MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kOther)
  } else {
    MAG_FAIL_HANDLE_AND_SET(true, talk.error = ErrorNo::kBroken)
  }
  return true;

  ERROR_HANDLE:
  events_driver_.RegEventDel(talk.fd);
  if (ErrorNo::kOk != talk.error) {
    CloseFd_(talk.fd);
    DEBUG("fail_handle_session_event_read[" << talk.fd << "]");
  }
  return false;
}

bool AgentSlave::HandleSessionEventWrite_(Talk& talk, EventCtx& event_ctx) {
  int ret = talk.protocol_write->Write(talk.fd);
  if (0==ret) {
    if (Talk::kWriteAndRead == talk.category) {
      time_t cur_time_ms = Time::GetCurrentMsec(false);
      ret = CheckEventCtxTimeout_(
          event_ctx,
          std::min(
              talk.endtime_ms - cur_time_ms,
              talk.remote->rtimeo_ms));
      MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kTimeout)

      MAG_BUG(talk.endtime_ms <= cur_time_ms)

      talk.status = Talk::kRead;
      ret = events_driver_.RegEvent(
          talk.fd,
          Driver::kModEvent, 
          Driver::kIn, 
          &event_ctx, 
          event_ctx.timeleft_ms);
      MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kOther)
    } else {
      MAG_BUG(Talk::kWriteOnly != talk.category)

      MAG_FAIL_HANDLE_AND_SET(true, talk.error = ErrorNo::kOk)
    }
  } else if (ret>0) {
    ret = CheckEventCtxTimeout_(event_ctx);
    MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kTimeout)

    ret = events_driver_.RegEvent(
        talk.fd, 
        Driver::kModEvent, 
        Driver::kOut, 
        &event_ctx, 
        event_ctx.timeleft_ms);
    MAG_FAIL_HANDLE_AND_SET(true!=ret, talk.error = ErrorNo::kOther)
  } else {
    MAG_FAIL_HANDLE_AND_SET(true, talk.error = ErrorNo::kBroken)
  }
  return true;

  ERROR_HANDLE:
  events_driver_.RegEventDel(talk.fd);
  if (ErrorNo::kOk != talk.error) {
    DEBUG("fail_handle_session_event_write[" << talk.error << "]");
    CloseFd_(talk.fd);
  }
  return false;
}

void AgentSlave::HandleMsgDestruct_(MsgDestruct& msg_destruct) {
  if (NULL == msg_destruct.big_cache) {
    for (size_t i=0; i < msg_destruct.small_cache.num; ++i) {
      conns_mgr_->FreeFd(
          *(msg_destruct.small_cache.small_cache[i].second),
          msg_destruct.small_cache.small_cache[i].first);
    }
  } else {
    for (size_t i=0; i < msg_destruct.big_cache->size(); ++i) {
      conns_mgr_->FreeFd(
          *((*msg_destruct.big_cache)[i].second),
          (*msg_destruct.big_cache)[i].first);
    }
    MAG_DELETE(msg_destruct.big_cache)
  }

  if (msg_destruct.fd_client <= 0) {
    --num_clients_keepalive_;
    PoolProtocols::FreeRead(msg_destruct.protocol_read);
    return;
  }

  int64_t timeleft_ms = std::min(
      ((int64_t)msg_destruct.fd_client_starttime_sec + 
        confs_->GetConfNormal().GetClientKeepaliveSec() -
        Time::GetCurrentSec(false) + 1) * 1000,
      confs_->GetConfNormal().GetRtimeoMs() );
  if (timeleft_ms<=0) {
    DEBUG("close_client_long_conns");
    CloseFdClient_(msg_destruct.fd_client);
    PoolProtocols::FreeRead(msg_destruct.protocol_read);
  } else {
    EventCtx* event_ctx = pool_event_ctxs_->Get();
    event_ctx->BuildForReadReq(
        msg_destruct.fd_client,
        msg_destruct.id_procedure,
        msg_destruct.fd_client_starttime_sec,
        *(msg_destruct.protocol_read),
        timeleft_ms);
    msg_destruct.protocol_read->Reset();

    bool ret = events_driver_.RegEvent(
        msg_destruct.fd_client,
        Driver::kAddEvent,
        Driver::kIn,
        event_ctx,
        event_ctx->timeleft_ms);
    if (!ret) {
      CloseFdClient_(msg_destruct.fd_client);
      PoolProtocols::FreeRead(msg_destruct.protocol_read);
      pool_event_ctxs_->Free(event_ctx);
      DEBUG("fail_reg_event");
    }
  }
}

int AgentSlave::HandleReadReq_(EventCtx& event_ctx) {
  ProtocolRead& protocol_read = *(event_ctx.data.read_req.protocol_read);
  int ret = protocol_read.Read(event_ctx.fd);
  if (0==ret) {
    ret = (protocol_read.Decode() ? 0 : -1); 
    MAG_FAIL_HANDLE(true)
  } else if (ret>0) {
    ret = CheckEventCtxTimeout_(event_ctx);
    MAG_FAIL_HANDLE(true!=ret)

    ret = events_driver_.RegEvent(
        event_ctx.fd, 
        Driver::kModEvent, 
        Driver::kIn, 
        &event_ctx, 
        event_ctx.timeleft_ms);
    MAG_FAIL_HANDLE(true!=ret)
    return 1;
  } else {
    MAG_FAIL_HANDLE(true)
  }

  ERROR_HANDLE:
  events_driver_.RegEventDel(event_ctx.fd);
  if (0!=ret && ProtocolRead::kEnd != ret) {
    DEBUG("fail_protocol_read");
  }
  return ret;
}

}
