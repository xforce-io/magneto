#include "../session.h"
#include "../model.h"
#include "../event_ctx.h"
#include "../confs/confs.h"

namespace magneto {

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
    (*talks_)[i].error = ErrorNo::kOk;
    ResetTalk_((*talks_)[i]);
    if (ErrorNo::kOk != (*talks_)[i].error) {
      ++num_talks_done_;
    }
  }
  return num_talks_done_ != talks_->size();
}

void Session::ResetTalk_(Talk& talk) {
  if (0 == talk.fd) {
    std::pair<int, bool> ret = conns_mgr_->GetFd(*(talk.service), talk.fail_remotes, talk.remote);
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
      return;
    }
  } else if (talk.fd < 0) {
    MAG_BUG(true)
    has_failure_=true;
    return;
  }

  EventCtx* event_ctx = pool_event_ctxs_->Get();
  event_ctx->BuildForSession(talk.fd, *this, talk.no_talk);

  Driver::Event event = 
    ( Talk::kRead == talk.status ? Driver::kIn : Driver::kOut );

  int64_t timeleft_ms = talk.endtime_ms - Time::GetCurrentMsec(false);
  if (unlikely(timeleft_ms<=0)) {
    talk.error = ErrorNo::kTimeout;
    has_failure_=true;
    return;
  }

  if (NULL != talk.remote) {
    event_ctx->timeleft_ms = std::min(
        timeleft_ms, 
        Talk::kConn == talk.status ? 
          confs_->GetConfNormal().GetCtimeoMs() :
          ( Talk::kRead == talk.status ? 
            talk.remote->rtimeo_ms :
            talk.remote->wtimeo_ms ) );
  } else {
    event_ctx->timeleft_ms = std::min(
        timeleft_ms, 
        Talk::kRead == talk.status ? 
          confs_->GetConfNormal().GetRtimeoMs() : 
          confs_->GetConfNormal().GetWtimeoMs() );
  }

  bool ret = events_driver_->RegEvent(
      talk.fd,
      Driver::kAddEvent,
      event,
      event_ctx,
      event_ctx->timeleft_ms);
  if (unlikely(!ret)) {
    WARN("fail_to_reg_event fd[" << talk.fd << "] errno[" << strerror(errno) << "]");
    IOHelper::Close(talk.fd);
    talk.fd=-1;
    talk.error = ErrorNo::kOther;
    has_failure_=true;
  }
}

}
