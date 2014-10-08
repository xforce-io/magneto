#pragma once

#include "scheduler.h"
#include "../msg.h"

namespace magneto {

class Confs;
class Agents;

class Schedulers {
 public: 
  bool Init(
      const Confs& confs,
      Agents& agents,
      const ReqHandler& req_handler,
      const RoutineItems* routine_items,
      void* args);

  inline bool SendToOneWorker(Msg& msg);

  bool Start();
  void Stop();
  virtual ~Schedulers();

 private:
  std::vector<Scheduler*> schedulers_req_;
  std::vector<Scheduler*> schedulers_routine_;
};

bool Schedulers::SendToOneWorker(Msg& msg) {
  MsgNewReq& msg_new_req = SCAST<MsgNewReq&>(msg);
  return schedulers_req_[msg_new_req.id_procedure % schedulers_req_.size()]->SendMsg(msg);
}

}
