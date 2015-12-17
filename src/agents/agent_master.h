#pragma once

#include "public.h"
#include "../msg.h"
#include "../model.h"

namespace xforce { namespace magneto {

class Confs;
class Schedulers;
class Agents;
class EventCtx;

class AgentMaster {
 public:
  typedef AgentMaster Self;
  typedef std::vector<Remote> Addrs;
  typedef EventsDriver<EventCtx> Driver;

 public:
  AgentMaster();
  bool Init(const Confs& confs, Schedulers& schedulers, Agents& agents);
  bool Start();

  void Stop();
  virtual ~AgentMaster();

 private:
  bool ListenOnAddr_(const ListenAddr& listen_addr);

  static void* Run_(void* args);
  void Process_();
  bool CheckEvents_();

 private:
  const Confs* confs_;
  Schedulers* schedulers_;
  Agents* agents_;

  Driver events_driver_;
  pthread_t tid_agent_master_;

  MsgReadReq tmp_msg_read_req_;

  bool end_;
};

}}
