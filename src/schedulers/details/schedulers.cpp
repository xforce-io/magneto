#include "../schedulers.h"
#include "../../confs/confs.h"

namespace xforce { namespace magneto {

bool Schedulers::Init(
    const Confs& confs,
    Agents& agents,
    const ReqHandler& req_handler,
    const RoutineItems* routine_items,
    void* args) {
  if (NULL!=req_handler) {
    for (size_t i=0; i < confs.GetConfNormal().GetNumSchedulers(); ++i) {
      XFC_NEW_DECL(scheduler, Scheduler, Scheduler)
      bool ret = scheduler->Init(confs, agents, req_handler, NULL, args);
      if (!ret) {
        XFC_DELETE(scheduler)
        return false;
      }
      schedulers_req_.push_back(scheduler);
    }
  }

  if (NULL!=routine_items) {
    for (size_t i=0; i < routine_items->size(); ++i) {
      XFC_NEW_DECL(scheduler, Scheduler, Scheduler)
      bool ret = scheduler->Init(confs, agents, NULL, &((*routine_items)[i]), args);
      if (!ret) {
        XFC_DELETE(scheduler)
        return false;
      }
      schedulers_routine_.push_back(scheduler);
    }
  }
  return true;
}

bool Schedulers::Start() {
  for (size_t i=0; i < schedulers_req_.size(); ++i) {
    if (!schedulers_req_[i]->Start()) {
      return false;
    }
  }

  for (size_t i=0; i < schedulers_routine_.size(); ++i) {
    if (!schedulers_routine_[i]->Start()) {
      return false;
    }
  }
  return true;
}

void Schedulers::Stop() {
  for (size_t i=0; i < schedulers_req_.size(); ++i) {
    schedulers_req_[i]->Stop();
  }

  for (size_t i=0; i < schedulers_routine_.size(); ++i) {
    schedulers_routine_[i]->Stop();
  }
}

Schedulers::~Schedulers() {
  Stop();
  for (size_t i=0; i < schedulers_req_.size(); ++i) {
    XFC_DELETE(schedulers_req_[i])
  }

  for (size_t i=0; i < schedulers_routine_.size(); ++i) {
    XFC_DELETE(schedulers_routine_[i])
  }
}

}}
