#include "../agent_master.h"

#include "../../event_ctx.h"
#include "../../confs/confs.h"
#include "../../schedulers/schedulers.h"
#include "../../agents/agents.h"

namespace magneto {

AgentMaster::AgentMaster() :
  tid_agent_master_(0),
  end_(false) {}

bool AgentMaster::Init(const Confs& confs, Schedulers& schedulers, Agents& agents) {
  confs_ = &confs;
  schedulers_ = &schedulers;
  agents_ = &agents;

  const ConfNormal::ListenAddrs& listen_addrs = confs_->GetConfNormal().GetListenAddrs();
  ConfNormal::ListenAddrs::const_iterator iter;
  for (iter = listen_addrs.begin(); iter != listen_addrs.end(); ++iter) {
    NOTICE("ready_listen_on[" 
        << inet_ntoa(iter->addr.addr.sin_addr)
        << "|" 
        << ntohs(iter->addr.addr.sin_port)
        << "]");
      
    bool ret = ListenOnAddr_(*iter);
    if (!ret) {
      FATAL("fail_accept_on_addr[" 
          << inet_ntoa(iter->addr.addr.sin_addr) 
          << "|" 
          << ntohs(iter->addr.addr.sin_port) 
          << "]");
      return false;
    }
  }
  return true;
}

bool AgentMaster::Start() {
  int ret = pthread_create(&tid_agent_master_, NULL, Run_, this);
  if (0!=ret) {
    FATAL("fail_spawn_agent_master_thread");
    return false;
  }
  return true;
}

void AgentMaster::Stop() {
  end_=true;
  if (0!=tid_agent_master_) {
    pthread_join(tid_agent_master_, NULL);
    tid_agent_master_=0;
  }

  std::vector<EventCtx*>* event_ctxs = events_driver_.ClearAll();
  for (size_t i=0; i < event_ctxs->size(); ++i) {
    MAG_DELETE((*event_ctxs)[i])
  }
}

AgentMaster::~AgentMaster() {
  Stop();
}

bool AgentMaster::ListenOnAddr_(const ListenAddr& listen_addr) {
  int fd = IOHelper::Listen(listen_addr.addr.addr, 20480);
  if (fd<=0) {
    FATAL("fail_listen ret[" << fd << "]");
    return false;
  }

  MAG_NEW_DECL(event_ctx, EventCtx, EventCtx)
  event_ctx->BuildForListen(fd, listen_addr);
  bool ret = events_driver_.RegEvent(fd, Driver::kAddEvent, Driver::kIn, event_ctx, -1);
  if (!ret) {
    MAG_DELETE(event_ctx)
    FATAL("fail_to_reg_event error[" << strerror(errno) << "]");
    return false;
  }
  return true;
}

void* AgentMaster::Run_(void* args) {
  prctl(PR_SET_NAME, "agent_master");

  Self& self = *(RCAST<Self*>(args));
  while (!(self.end_)) {
    self.Process_();
  }
  self.Process_();
  return NULL;
}

void AgentMaster::Process_() {
  bool ret=true;
  size_t round=0;
  while (ret) {
    ret = CheckEvents_();
    ++round;
  }

  if (1==round) {
    usleep(1);
  }
}

bool AgentMaster::CheckEvents_() {
  int num_events_ready = events_driver_.Wait(0);
  if (num_events_ready>0) {
    int ret;
    Driver::Event event = Driver::kErr;
    EventCtx* event_ctx=NULL;
    for (int i=0; i<num_events_ready; ++i) {
      events_driver_.CheckReadyEvent(i, event, event_ctx);

      switch (event_ctx->category) {
        case EventCtx::kListen : {
          int new_fd=0;
          MAG_FAIL_HANDLE(Driver::kIn != event && (ret=-1))

          new_fd = IOHelper::Accept(event_ctx->fd);
          MAG_FAIL_HANDLE(-1==new_fd && (ret=-2))

          if (!agents_->ClientsNotFull()) {
            static size_t i=0;
            if (i++%10000) {
              WARN("too_many_connections_from_client close");
            }
            IOHelper::Close(new_fd);
            continue;
          }

          tmp_msg_read_req_.BuildForReadReq(new_fd, *(event_ctx->data.listen.listen_addr));
          ret = agents_->SendMsg(tmp_msg_read_req_.id_procedure, tmp_msg_read_req_);
          MAG_FAIL_HANDLE(true!=ret && (ret=-3))
          break;

          ERROR_HANDLE:
          if (new_fd>0) IOHelper::Close(new_fd);
          WARN("fail_accept ret[" 
              << ret
              << "] addr[" 
              << event_ctx->data.listen.listen_addr->addr.addr.sin_addr.s_addr
              << "|" 
              << event_ctx->data.listen.listen_addr->addr.addr.sin_port
              << "] error[" 
              << strerror(errno)
              << "]");
          break;
        } 
        default : {
          MAG_BUG(true)
          break;
        }
      }
    }
    return true;
  } else {
    if (num_events_ready<0) {
      FATAL("fail_wait_for_events reason[" << strerror(errno) << "]");
    }
    return false;
  }
}

}
