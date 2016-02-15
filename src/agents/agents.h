#pragma once

#include "public.h"
#include "../model.h"

namespace xforce { namespace magneto {

class Confs;
class Schedulers;
class Msg;
class AgentMaster;
class AgentSlave;

class Agents {
 public:
  Agents();

  bool Init(const Confs& confs, Schedulers& schedulers);
  bool Start();

  inline bool SendMsg(int id, Msg& msg);
  inline void SendMsgWithRetry(int id, Msg& msg);
  inline void BroadcastWithRetry(Msg& msg);
  inline bool ClientsNotFull() const;

  void Stop();
  virtual ~Agents();

 private:
  //const
  const Confs* confs_;
  ///

  AgentMaster* agent_master_;
  std::vector<AgentSlave*> agent_slaves_; 
};

}}

#include "agent_slave.h"
#include "../confs/confs.h"

namespace xforce { namespace magneto {

void Agents::BroadcastWithRetry(Msg& msg) {
  for (size_t i=0; i < agent_slaves_.size(); ++i) {
    SendMsgWithRetry(i, msg);
  }
}

bool Agents::ClientsNotFull() const {
  int num_clients_keepalive=0;
  for (size_t i=0; i < agent_slaves_.size(); ++i) {
    num_clients_keepalive += agent_slaves_[i]->GetNumClientsKeepalive();
  }
  return num_clients_keepalive < SCAST<int64_t>(confs_->GetConfNormal().GetMaxNumClientsKeepalive());
}

bool Agents::SendMsg(int id, Msg& msg) {
  return agent_slaves_[id % agent_slaves_.size()]->SendMsg(msg);
}

void Agents::SendMsgWithRetry(int id, Msg& msg) {
  if (likely(SendMsg(id, msg))) return;

  static const size_t kWarnThreshold=1000;
  size_t times=0;
  while (!SendMsg(id, msg)) {
    ++times;
    if (0 == times%kWarnThreshold) {
      WARN("agents_send_msg_retry[" << times << "]");
      usleep(10);
    }
  }
}

}}
