#pragma once

#include "public.h"
#include "model.h"
#include "handlers.h"
#include "ctx_helper.h"

namespace xforce { namespace magneto {

class Talk;
class Remote;
class MsgSession;
class Agents;
class Scheduler;
class MsgNewReq;
class Service;
class ProtocolRead;
class MsgDestruct;

class BizProcedure {
 public: 
  typedef PoolObjs< std::vector<Talk> > PoolTalks;
  typedef std::tr1::unordered_map< size_t, std::pair<int, const Remote*> > ServiceCache;
  typedef std::vector<std::vector<Talk>*> TmpPoolTalks;

 public:
  BizProcedure() {}
  BizProcedure(
      Agents& agents, 
      size_t size_stack, 
      void* args,
      Scheduler& scheduler);

  inline void Procedure(const MsgNewReq& msg_new_req, ucontext_t* ctx);

  inline static bool SetCurrentBizProcedure(BizProcedure& biz_procedure);
  inline static BizProcedure* GetCurrentBizProcedure();
  
  void SetMsgSession(const MsgSession& msg_session) { msg_session_ = &msg_session; }
  const MsgSession* GetMsgSession() const { return msg_session_; }

  void SetFdClientInvalid() { fd_client_=-1; }
  void SetWriteBackCalled() { write_back_called_=true; }

  ucontext_t& GetCtx() { return *ctx_; }
  inline std::vector<Talk>* GetTalk();
  inline void FreeTalks();
  inline void GetFdFromServiceCache(const Service& service, int& fd, const Remote*& remote);
  inline void InsertFdIntoServiceCache(const Service& service, int fd, const Remote& remote);
  inline void InvalidFdInServiceCache(const Service& service);

  Scheduler* GetScheduler() { return scheduler_; }
  int GetIdProcedure() const { return id_procedure_; }
  int GetFdClient() const { return fd_client_; }
  time_t GetFdClientStarttimeSec() const { return fd_client_starttime_sec_; }
  size_t GetVersionConfServices() const { return version_conf_services_; }
  const ServiceCache& GetServiceCache() const { return service_cache_; }
  const ProtocolRead* GetProtocolRead() const { return protocol_read_; }

  virtual ~BizProcedure();
 
 private:
  bool Init_(); 
  inline void Reset_();
  inline static void Process_(int this_high, int this_low); 
 
 private:
  //const
  Agents* agents_;
  size_t size_stack_; 
  void* args_;
  Scheduler* scheduler_;
  ///

  PoolTalks* pool_talks_;
  ServiceCache service_cache_;
  TmpPoolTalks tmp_pool_talks_;

  char* stack_;
  ucontext_t* ctx_;
  int id_procedure_;
  int fd_client_;
  time_t fd_client_starttime_sec_;
  ProtocolRead* protocol_read_;
  size_t version_conf_services_;
  const MsgSession* msg_session_;
  bool write_back_called_;

  static ThreadPrivacy thread_privacy_;

  MsgDestruct* tmp_msg_destruct_;

  bool init_;
};

}}

#include "msg.h"
#include "schedulers/scheduler.h"
#include "agents/agents.h"
#include "protocols/pool_protocols.h"

namespace xforce { namespace magneto {

void BizProcedure::Procedure(const MsgNewReq& msg_new_req, ucontext_t* ctx) {
  XFC_RAII_INIT()

  id_procedure_ = msg_new_req.id_procedure;
  fd_client_ = msg_new_req.fd_client;
  fd_client_starttime_sec_ = msg_new_req.fd_client_starttime_sec;
  protocol_read_ = msg_new_req.protocol_read;

  int this_high, this_low;
  CtxHelper::PtrToInts<BizProcedure>(this, this_high, this_low);
  ctx_->uc_stack.ss_sp = stack_;
  ctx_->uc_link = ctx;
  makecontext(ctx_, (void (*)(void))Process_, 2, this_high, this_low);
}

bool BizProcedure::SetCurrentBizProcedure(BizProcedure& biz_procedure) {
  BizProcedure** slot_biz_procedure = thread_privacy_.Get<BizProcedure*>();
  if (unlikely(NULL==slot_biz_procedure)) return false;

  *slot_biz_procedure = &biz_procedure;
  return true;
}

BizProcedure* BizProcedure::GetCurrentBizProcedure() {
  BizProcedure** slot_biz_procedure = thread_privacy_.Get<BizProcedure*>(0, NULL);
  return NULL!=slot_biz_procedure ? *slot_biz_procedure : NULL;
}

std::vector<Talk>* BizProcedure::GetTalk() {
  std::vector<Talk>* result = pool_talks_->Get();
  tmp_pool_talks_.push_back(result);
  return result;
}

void BizProcedure::FreeTalks() {
  for (TmpPoolTalks::iterator iter = tmp_pool_talks_.begin(); iter != tmp_pool_talks_.end(); ++iter) {
    const std::vector<Talk>& talks = (**iter);
    for (size_t i=0; i < talks.size(); ++i) {
      const Talk& talk = talks[i];
      if (NULL != talk.protocol_write) {
        PoolProtocols::FreeWrite(talk.protocol_write);
      }

      if (NULL != talk.protocol_read) {
        PoolProtocols::FreeRead(talk.protocol_read);
      }
    }
    pool_talks_->Free(*iter);
  }
  tmp_pool_talks_.clear();
}

void BizProcedure::GetFdFromServiceCache(const Service& service, int& fd, const Remote*& remote) {
  ServiceCache::const_iterator iter = service_cache_.find(service.no);
  if (service_cache_.end() == iter) {
    fd=0;
  } else {
    fd = iter->second.first;
    remote = iter->second.second;
    service_cache_.erase(iter);
  }
}

void BizProcedure::InsertFdIntoServiceCache(const Service& service, int fd, const Remote& remote) {
  service_cache_.insert(std::make_pair(service.no, std::make_pair(fd, &remote)));
}

void BizProcedure::InvalidFdInServiceCache(const Service& service) {
  service_cache_.erase(service.no);
}

void BizProcedure::Reset_() {
  TRACE("BuildForDestruct fd[" << GetFdClient() << "]");

  XFC_ASSERT(NULL!=protocol_read_)
  tmp_msg_destruct_->BuildForDestruct(
      GetIdProcedure(),
      GetFdClient(), 
      GetFdClientStarttimeSec(),
      protocol_read_,
      *this);
  agents_->SendMsgWithRetry(GetIdProcedure(), *tmp_msg_destruct_);
  FreeTalks();
  service_cache_.clear();
}

void BizProcedure::Process_(int this_high, int this_low) {
  BizProcedure* biz_procedure = CtxHelper::IntsToPtr<BizProcedure>(this_high, this_low);
  if (likely(biz_procedure->scheduler_->IsReqScheduler())) {
    biz_procedure->scheduler_->GetReqHandler()(
        *(biz_procedure->protocol_read_), 
        biz_procedure->args_);
    biz_procedure->Reset_();
  } else {
    biz_procedure->scheduler_->GetRoutineHandler()(biz_procedure->args_);
  }
  biz_procedure->scheduler_->FreeBizProcedure(*biz_procedure);
}

}}
