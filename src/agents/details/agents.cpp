#include "../agents.h"
#include "../agent_master.h"

namespace xforce { namespace magneto {

Agents::Agents() :
  agent_master_(NULL) {}

bool Agents::Init(const Confs& confs, Schedulers& schedulers) {
  confs_ = &confs;

  for (size_t i=0; i < confs_->GetConfNormal().GetNumAgents(); ++i) {
    XFC_NEW_DECL(agent_slave, AgentSlave, AgentSlave)
    bool ret = agent_slave->Init(confs, schedulers);
    if (!ret) {
      XFC_DELETE(agent_slave)
      return false;
    }

    agent_slaves_.push_back(agent_slave);
  }

  XFC_NEW(agent_master_, AgentMaster())
  return agent_master_->Init(confs, schedulers, *this);
}

bool Agents::Start() {
  for (size_t i=0; i < agent_slaves_.size(); ++i) {
    bool ret = agent_slaves_[i]->Start();
    if (!ret) {
      FATAL("fail_start_agent_slave");
      return false;
    }
  }

  bool ret = agent_master_->Start();
  if (!ret) {
    FATAL("fail_start_agent_master");
    return false;
  }
  return true;
}

void Agents::Stop() {
  agent_master_->Stop();
  for (size_t i=0; i < agent_slaves_.size(); ++i) {
    agent_slaves_[i]->Stop();
  }
}

Agents::~Agents() {
  Stop();
  for (size_t i=0; i < agent_slaves_.size(); ++i) {
    XFC_DELETE(agent_slaves_[i])
  }
  XFC_DELETE(agent_master_)
}

}}
