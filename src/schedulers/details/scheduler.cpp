#include "../scheduler.h"
#include "../../biz_procedure.h"
#include "../../ctx_helper.h"
#include "../../confs/confs.h"

namespace xforce { namespace magneto {

Scheduler::Scheduler() :
  default_notifier_(NULL),
  pool_biz_procedure_(NULL),
  pool_msgs_(NULL),
  tid_scheduler_(0),
  end_(false) {}

bool Scheduler::Init(
    const Confs& confs, 
    Agents& agents,
    const ReqHandler& req_handler,
    const RoutineItem* routine_item,
    void* args) {
  size_stack_ = confs.GetConfNormal().GetSizeStack();
  if (NULL!=req_handler) {
    req_handler_ = req_handler;
    is_req_scheduler_=true;
  } else {
    routine_handler_ = routine_item->first;
    num_case_routine_item_ = routine_item->second;
    is_req_scheduler_=false;
  }

  XFC_NEW(default_notifier_, DefaultNotifier)

  int ret = default_notifier_->Init() 
    && mailbox_.Init(
        confs.GetConfNormal().GetSizeSchedulerMailbox(), 
        Msg::kMaxSizeMsg, 
        kThreholdSenderNotify, 
        default_notifier_);
  if (true!=ret) {
    FATAL("fail_init_mailbox");
    return false;
  }

  XFC_NEW(pool_biz_procedure_, PoolObjsInit<BizProcedure>(
      BizProcedure(agents, size_stack_, args, *this)))
  XFC_NEW(pool_msgs_, PoolObjs<Msg> [Msg::kNumMsgCategories])
  return true;
}

bool Scheduler::Start() {
  int ret = pthread_create(&tid_scheduler_, NULL, Run_, this);
  if (0!=ret) {
    FATAL("fail_start_scheduler_thread");
    return false;
  }
  return true;
}

void Scheduler::Stop() {
  end_=true;
  if (0!=tid_scheduler_) {
    pthread_join(tid_scheduler_, NULL);
    tid_scheduler_=0;
  }
}

Scheduler::~Scheduler() {
  Stop();
  XFC_DELETE_ARRAY(pool_msgs_)
  XFC_DELETE(pool_biz_procedure_)
  XFC_DELETE(default_notifier_)
}

void* Scheduler::Run_(void* args) {
  Scheduler* scheduler = RCAST<Scheduler*>(args);
  if (scheduler->IsReqScheduler()) {
    prctl(PR_SET_NAME, "req_scheduler");
  } else {
    prctl(PR_SET_NAME, "routine_scheduler");

    MsgNewReq msg_new_req;
    for (size_t i=0; i < scheduler->num_case_routine_item_; ++i) {
      int ret = scheduler->SendMsg(msg_new_req);
      if (!ret) {
        FATAL("fail_start_routine_thread");
        return NULL;
      }
    }
  }

  DEBUG("thread[Scheduler::Run_] start");
  while (!(scheduler->end_)) {
    scheduler->Process_();
  }
  scheduler->Process_();
  DEBUG("thread[Scheduler::Run_] stop");
  return NULL;
}

void Scheduler::Process_() {
  bool round=false;
  while (true) {
    Mailbox::Msg* msg = mailbox_.ReceiveMsg(0);
    if (NULL==msg) { break; }

    round=true;

    const Msg& cur_msg = *RCAST<const Msg*>(msg->content);
    Msg* new_msg = pool_msgs_[cur_msg.category].Get();
    if (unlikely(cur_msg.category != new_msg->category)) {
      XFC_DELETE(new_msg)
      switch (cur_msg.category) {
        case Msg::kConfig :
          XFC_NEW(new_msg, MsgConfig)
          break;
        case Msg::kReadReq :
          XFC_NEW(new_msg, MsgReadReq)
          break;
        case Msg::kNewReq :
          XFC_NEW(new_msg, MsgNewReq)
          break;
        case Msg::kSession :
          XFC_NEW(new_msg, MsgSession)
          break;
        case Msg::kDestruct :
          XFC_NEW(new_msg, MsgDestruct)
          break;
        default :
          XFC_BUG(true)
      }
    }
    new_msg->Copy(cur_msg);

    task_queue_.push_back(new_msg);
    mailbox_.MsgConsumed();
  }

  if (!round && task_queue_.empty()) {
    usleep(10);
    return;
  }

  for (size_t i=0; i<kMaxTaskProcessedOneTime; ++i) {
    Msg* msg = SelectTask_();
    if (NULL==msg) return;

    if (Msg::kSession == msg->category) {
      MsgSession& msg_session = *SCAST<MsgSession*>(msg);
      msg_session.biz_procedure->SetMsgSession(msg_session);

      int ret = BizProcedure::SetCurrentBizProcedure(*(msg_session.biz_procedure));
      XFC_FAIL_HANDLE_FATAL(true!=ret, "fail_set_cur_biz_procedure")

      ret = swapcontext(&ctx_, msg_session.biz_ctx);
      XFC_FAIL_HANDLE_FATAL(0!=ret, "fail_swapcontext_in_scheduler")
    } else if (Msg::kNewReq == msg->category) {
      MsgNewReq& msg_new_req = *SCAST<MsgNewReq*>(msg);
      BizProcedure* biz_procedure = pool_biz_procedure_->Get();
      biz_procedure->Procedure(msg_new_req, &ctx_);

      int ret = BizProcedure::SetCurrentBizProcedure(*biz_procedure);
      XFC_FAIL_HANDLE_FATAL(true!=ret, "fail_set_cur_biz_procedure")

      ret = swapcontext(&ctx_, &(biz_procedure->GetCtx()));
      XFC_FAIL_HANDLE_FATAL(0!=ret, "fail_swapcontext_in_scheduler")
    } else {
      XFC_BUG(true)
    }
    pool_msgs_[msg->category].Free(msg);
    ConsumeTask_();
    continue;

    ERROR_HANDLE:
    pool_msgs_[msg->category].Free(msg);
    ConsumeTask_();
  }
}

}}
