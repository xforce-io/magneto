#include "../session.h"
#include "../model.h"
#include "../event_ctx.h"
#include "../confs/confs.h"

namespace xforce { namespace magneto {

Session::Session(
    const Confs& confs,
    Driver& events_driver, 
    ConnsMgr& conns_mgr,
    PoolObjs<EventCtx>& pool_event_ctxs) :
  confs_(&confs),
  events_driver_(&events_driver),
  conns_mgr_(&conns_mgr),
  pool_event_ctxs_(&pool_event_ctxs) {}

bool Session::Reset(ucontext_t& biz_ctx, BizProcedure& biz_procedure, std::vector<Talk>& talks) {
  biz_ctx_ = &biz_ctx;
  biz_procedure_ = &biz_procedure;
  talks_ = &talks;
  num_talks_done_=0;
  has_failure_=false;

  for (size_t i=0; i < talks_->size(); ++i) {
    if (Talk::kEnd == (*talks_)[i].status) {
      //some buf==NULL in ParaTalks and Write
      ++num_talks_done_;
      continue;
    }

    ResetTalk_((*talks_)[i], false);
    if (ErrorNo::kOk != (*talks_)[i].error) {
      ++num_talks_done_;
    }
  }
  return num_talks_done_ != talks_->size();
}

void Session::ResetTalk_(Talk& talk, bool is_retry) {
  if (!GetFdForTalk_(talk)) {
    return;
  }

  if (is_retry && NULL != talk.protocol_write) {
    talk.protocol_write->Reset(*(talk.buf));
    bool ret = talk.protocol_write->Encode();
    if (!ret) {
      talk.error = ErrorNo::kEncode;
      has_failure_=true;
      DEBUG("fail_encode_write_buf");
      return;
    }
  }

  EventCtx* event_ctx = pool_event_ctxs_->Get();
  event_ctx->BuildForSession(talk.fd, *this, talk.no_talk);

  if (!SetTimeleftForEvent_(*event_ctx, talk)) {
    return;
  }

  bool ret = events_driver_->RegEvent(
      talk.fd,
      Driver::kAddEvent,
      Talk::kRead == talk.status ? Driver::kIn : Driver::kOut,
      event_ctx,
      event_ctx->timeleft_ms);
  if (unlikely(!ret)) {
    IOHelper::Close(talk.fd);
    talk.fd = -1;
    talk.error = ErrorNo::kOther;
    has_failure_=true;
    WARN("fail_to_reg_event fd[" << talk.fd << "] errno[" << strerror(errno) << "]");
  }
}

bool Session::GetFdForTalk_(Talk& talk) {
  if (0 == talk.fd) {
    std::pair<int, bool> ret;
    if (NULL == talk.remote) {
      ret = conns_mgr_->GetFd(*(talk.service), talk.fail_remotes, talk.remote);
    } else {
      //remote io, no need to choose remote
      ret = conns_mgr_->GetFd(talk.remote->name);
    }

    if (ret.first > 0) {
      talk.fd = ret.first;
      if (!(ret.second)) {
        talk.status = Talk::kConn;
      } else {
        talk.status = (Talk::kReadOnly == talk.category ? Talk::kRead : Talk::kWrite);
      }
    } else {
      talk.fd = -1;
      talk.error = ErrorNo::kBroken;
      has_failure_=true;
      WARN("fail_get_fd");
      return false;
    }
  } else if (talk.fd < 0) {
    XFC_BUG(true)
    has_failure_=true;
    return false;
  } // talk.fd > 0 indicates that fd already got from biz cache
  return true;
}

bool Session::SetTimeleftForEvent_(EventCtx& event_ctx, Talk& talk) {
  int64_t timeleft_ms = talk.endtime_ms - Time::GetCurrentMsec(false);
  if (unlikely(timeleft_ms<=0)) {
    talk.error = ErrorNo::kTimeout;
    has_failure_=true;
    DEBUG("timeout_when_make_session timeleft_ms[" << timeleft_ms << "]");
    return false;
  }

  if (NULL != talk.remote) {
    event_ctx.timeleft_ms = std::min(
        timeleft_ms, 
        Talk::kConn == talk.status ? 
          confs_->GetConfNormal().GetCtimeoMs() :
          ( Talk::kRead == talk.status ? 
            talk.remote->rtimeo_ms :
            talk.remote->wtimeo_ms ) );
  } else {
    //if write back
    event_ctx.timeleft_ms = std::min(
        timeleft_ms, 
        Talk::kRead == talk.status ? 
          confs_->GetConfNormal().GetRtimeoMs() : 
          confs_->GetConfNormal().GetWtimeoMs() );
  }
  return true;
}

}}
