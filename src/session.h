#pragma once

#include "public.h"
#include "agents/agent_slave.h"

namespace xforce { namespace magneto {

class Confs;
class EventCtx;
class ConnsMgr;
class BizProcedure;
class Talk;

class Session {
 private:
  typedef AgentSlave::Driver Driver; 

 public:
  Session() {}
  Session(
      const Confs& confs,
      Driver& events_driver, 
      ConnsMgr& conns_mgr, 
      PoolObjs<EventCtx>& pool_event_ctxs);

  bool Reset(ucontext_t& biz_ctx, BizProcedure& biz_procedure, std::vector<Talk>& talks);

  ucontext_t* GetBizCtx() { return biz_ctx_; }
  BizProcedure* GetBizProcedure() { return biz_procedure_; }
  Talk& GetTalk(size_t no_talk) { return (*talks_)[no_talk]; }
  std::vector<Talk>* GetTalks() { return talks_; }
  bool HasFailure() const { return has_failure_; }

  /*
   * @return :
   *  true  : finish
   *  false : not finish
   */
  inline bool FinishTalk(bool succ, Talk& talk);
  
 private:
  void ResetTalk_(Talk& talk, bool is_retry);

  /*
   * @return : 
   *    true => succ
   *    false=> fail
   */
  bool GetFdForTalk_(Talk& talk);

  /*
   * @return :
   *    true => succ
   *    false=> timeout happen, fail
   */
  bool SetTimeleftForEvent_(EventCtx& event_ctx, Talk& talk);

 private:
  //const
  const Confs* confs_;
  Driver* events_driver_;
  ConnsMgr* conns_mgr_;
  PoolObjs<EventCtx>* pool_event_ctxs_;
  //

  ucontext_t* biz_ctx_;
  BizProcedure* biz_procedure_;
  std::vector<Talk>* talks_;
  size_t num_talks_done_;
  bool has_failure_;
};

}}

#include "conns_mgr/conns_mgr.h"

namespace xforce { namespace magneto {

bool Session::FinishTalk(bool succ, Talk& talk) {
  ++num_talks_done_;
  XFC_BUG(num_talks_done_ > talks_->size())

  /* NULL == talk.remote when WriteBack */
  if (NULL != talk.remote) {
    conns_mgr_->ReportStatus(talk.remote->name, succ);
    if (!succ) {
      talk.fail_remotes.push_back(*(talk.remote));
    }
  }

  if (unlikely(!succ)) {
    if (0 != talk.retry) {
      --talk.retry;
      if (talk.fd>0) IOHelper::Close(talk.fd);

      talk.fd = 0;
      talk.error = ErrorNo::kOk;
      DEBUG("retry");
      ResetTalk_(talk, true);
      if (ErrorNo::kOk == talk.error) {
        //retry happens
        --num_talks_done_;
      } else {
        has_failure_=true;
      }
    } else {
      has_failure_=true;
    }
  }
  return num_talks_done_ == talks_->size();
}

}}
