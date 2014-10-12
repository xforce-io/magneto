#pragma once

#include "error_no.h"
#include "public/common.h"
#include "public/buffer.hpp"
#include "public/time/time.h"
#include "public/gmonitor.h"
#include "public/mem_profile.h"
#include "protocols/protocols.h"
#include "handlers.h"

namespace magneto {

class MagnetoBase;

class Magneto {
 public:
  Magneto();

  bool Init(
      const std::string& conf_service_dir,
      const ReqHandler& req_handler,
      const RoutineItems* routine_items,
      void* args,
      bool& end);

  const std::vector<std::string>* GetServiceNames(const std::string& services_set);

  int Write(
      IN const std::string& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Errors& errors);

  int Read(
      IN const std::string& services_set,
      IN time_t timeo_ms,
      OUT Responses& responses);

  int Talks(
      IN const std::string& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Responses& responses);

  int Write(
      const std::string& service, 
      const Buf& buf, 
      time_t timeo_ms); 

  int Read(
      IN const std::string& service, 
      IN time_t timeo_ms, 
      OUT ProtocolRead*& protocol_read);

  int SimpleTalk(
      IN const std::string& service,
      IN const Buf& buf,
      IN time_t timeo_ms,
      OUT ProtocolRead*& protocol_read);

  int WriteBack(const Buf& buf, time_t timeo_ms);

  void FreeTalks();

  void Stop();

  virtual ~Magneto();
 
 private:
  MagnetoBase* magneto_base_; 
};

}
