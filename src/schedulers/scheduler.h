#pragma once

#include "public.h"
#include "../handlers.h"
#include "../msg.h"

namespace magneto {

class Confs;
class Agents;
class BizProcedure;

class Scheduler {
 public:
  typedef Scheduler Self;
  typedef MPSCFixedPipe<bool> Mailbox; 
  typedef std::list<Msg*> TaskQueue;
  typedef std::pair<RoutineHandler, size_t> RoutineItem;
 
 private:
  static const size_t kMaxTaskProcessedOneTime=100;
  static const size_t kThreholdSenderNotify=0;

 public:
  Scheduler();

  bool Init(
      const Confs& confs, 
      Agents& agents,
      const ReqHandler& req_handler,
      const RoutineItem* routine_item,
      void* args);

  bool Start();

  inline bool SendMsg(Msg& msg);
  bool IsReqScheduler() const { return is_req_scheduler_; }
  ReqHandler GetReqHandler() const { return req_handler_; }
  RoutineHandler GetRoutineHandler() const { return routine_handler_; }
  Mailbox& GetMailbox() { return mailbox_; }
  ucontext_t& GetCtx() { return ctx_; }
  inline void FreeBizProcedure(BizProcedure& biz_procedure);

  void Stop();
  virtual ~Scheduler();

 private:
  static void* Run_(void* args);
  void Process_();
  inline Msg* SelectTask_();
  inline void ConsumeTask_();

 private:
  size_t size_stack_;
  bool is_req_scheduler_;
  ReqHandler req_handler_;
  RoutineHandler routine_handler_;
  size_t num_case_routine_item_;

  ucontext_t ctx_;
  DefaultNotifier* default_notifier_;
  Mailbox mailbox_; 
  TaskQueue task_queue_;
  PoolObjsInit<BizProcedure>* pool_biz_procedure_;
  PoolObjs<Msg>* pool_msgs_;

  pthread_t tid_scheduler_;

  bool end_;
};

}

#include "../biz_procedure.h"

namespace magneto {

bool Scheduler::SendMsg(Msg& msg) {
  return GetMailbox().SendMsg(true, RCAST<const char*>(&msg), msg.size);
}

void Scheduler::FreeBizProcedure(BizProcedure& biz_procedure) {
  pool_biz_procedure_->Free(&biz_procedure);
}

Msg* Scheduler::SelectTask_() {
  if (unlikely(task_queue_.empty())) {
    return NULL;
  }
  return task_queue_.front();
}

void Scheduler::ConsumeTask_() {
  task_queue_.pop_front();
}

}
