#pragma once

#include "public.h"
#include "handlers.h"

namespace xforce { namespace magneto {

class Protocol;
class Service;
class Confs;
class Schedulers;
class Agents;
class IOBasic;

class MagnetoBase {
 public:
  MagnetoBase();

  bool Init(
      const std::string& conf_service_dir,
      const ReqHandler& req_handler,
      const RoutineItems* routine_items,
      void* args,
      bool& end);

  inline const std::vector<std::string>* GetServiceNames(const std::string& services_set);

  inline int Write(
      IN const std::string& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Errors& errors);

  inline int Read(
      IN const std::string& services_set,
      IN time_t timeo_ms,
      OUT Responses& responses);

  inline int Talks(
      IN const std::string& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Responses& responses);

  inline int Write(
      const std::string& service, 
      const Buf& buf, 
      time_t timeo_ms); 

  inline int Read(
      IN const std::string& service, 
      IN time_t timeo_ms, 
      OUT ProtocolRead*& protocol_read); 

  inline int SimpleTalk(
      IN const std::string& service,
      IN const Buf& buf,
      IN time_t timeo_ms,
      OUT ProtocolRead*& protocol_read);

  inline int WriteBack(const Buf& buf, time_t timeo_ms);

  inline void FreeTalks();
  
  void Stop();

  virtual ~MagnetoBase();

 private:
  void InitSignals_();

 private:
  bool* end_;
  
  Confs* confs_;
  Schedulers* schedulers_;
  Agents* agents_;
  IOBasic* io_basic_;
};

}}

#include "biz_procedure.h"
#include "io_basic/io_basic.h"

namespace xforce { namespace magneto {

const std::vector<std::string>* MagnetoBase::GetServiceNames(const std::string& services_set) {
  return confs_->GetConfServices()->GetServiceNames(services_set);
}

int MagnetoBase::Write(
    const std::string& services_set_name,
    const Bufs& bufs,
    time_t timeo_ms,
    Errors& errors) {
  const ServicesSet* services_set = confs_->GetConfServices()->GetServicesSet(services_set_name);
  if (unlikely(NULL==services_set)) return ErrorNo::kNotFound;

  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->Write(*biz_procedure, *services_set, bufs, timeo_ms, errors);
}

int MagnetoBase::Read(
    const std::string& services_set_name,
    time_t timeo_ms,
    Responses& responses) {
  const ServicesSet* services_set = confs_->GetConfServices()->GetServicesSet(services_set_name);
  if (unlikely(NULL==services_set)) return ErrorNo::kNotFound;

  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->Read(*biz_procedure, *services_set, timeo_ms, responses);
}

int MagnetoBase::Talks(
    const std::string& services_set_name,
    const Bufs& bufs,
    time_t timeo_ms,
    Responses& responses) {
  const ServicesSet* services_set = confs_->GetConfServices()->GetServicesSet(services_set_name);
  if (unlikely(NULL==services_set)) return ErrorNo::kNotFound;

  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->Talks(*biz_procedure, *services_set, bufs, timeo_ms, responses);
}

int MagnetoBase::Write(const std::string& service_name, const Buf& buf, time_t timeo_ms) {
  const Service* service = confs_->GetConfServices()->GetService(service_name);
  if (unlikely(NULL==service)) return ErrorNo::kNotFound; 

  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->Write(*biz_procedure, *service, buf, timeo_ms);
}

int MagnetoBase::Read(const std::string& service_name, time_t timeo_ms, ProtocolRead*& protocol_read) {
  const Service* service = confs_->GetConfServices()->GetService(service_name);
  if (unlikely(NULL==service)) return ErrorNo::kNotFound; 

  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->Read(*biz_procedure, *service, timeo_ms, protocol_read);
}

int MagnetoBase::SimpleTalk(
    const std::string& service_name,
    const Buf& buf,
    time_t timeo_ms,
    ProtocolRead*& protocol_read) {
  const Service* service = confs_->GetConfServices()->GetService(service_name);
  if (unlikely(NULL==service)) return ErrorNo::kNotFound; 

  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->SimpleTalk(*biz_procedure, *service, buf, timeo_ms, protocol_read);
}

int MagnetoBase::WriteBack(const Buf& buf, time_t timeo_ms) {
  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->WriteBack(*biz_procedure, buf, timeo_ms);
}

void MagnetoBase::FreeTalks() {
  BizProcedure* biz_procedure = BizProcedure::GetCurrentBizProcedure();
  return io_basic_->FreeTalks(*biz_procedure);
}

}}
