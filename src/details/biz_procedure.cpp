#include "../biz_procedure.h"
#include "../schedulers/scheduler.h"
#include "../confs/versioned_conf_services.h"
#include "../agents/agents.h"

namespace magneto {

ThreadPrivacy BizProcedure::thread_privacy_;

BizProcedure::BizProcedure(
    Agents& agents,
    size_t size_stack, 
    void* args,
    Scheduler& scheduler) :
  agents_(&agents),
  size_stack_(size_stack),
  args_(args),
  scheduler_(&scheduler),
  pool_talks_(NULL),
  stack_(NULL),
  ctx_(NULL),
  msg_session_((const MsgSession*)0x01),
  tmp_msg_destruct_(NULL),
  init_(false) {}

BizProcedure::~BizProcedure() {
  MAG_DELETE(tmp_msg_destruct_)
  MAG_DELETE(ctx_)
  MAG_FREE(stack_)
  MAG_DELETE(pool_talks_)
}

bool BizProcedure::Init_() {
  MAG_NEW(pool_talks_, PoolTalks(1, 1024, false, true, 1))
  MAG_NEW(ctx_, ucontext_t)
  MAG_MALLOC(stack_, char*, size_stack_)
  MAG_NEW(tmp_msg_destruct_, MsgDestruct)

  int ret = getcontext(ctx_);
  if (0!=ret) return false;

  ctx_->uc_stack.ss_size = size_stack_;
  ctx_->uc_link = &(scheduler_->GetCtx());

  init_=true;
  return true;
}

}
