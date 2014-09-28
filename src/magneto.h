#pragma once

#include "error_no.h"
#include "public/common.h"
#include "public/buffer.hpp"
#include "public/time/time.h"
#include "protocols/protocols.h"
#include "handlers.h"

namespace magneto {

class MagnetoBase;

class Magneto {
 public:
  typedef std::pair<const char*, int> Buf;
  typedef std::vector<const Buf*> Bufs;
  typedef std::pair<RoutineHandler, size_t> RoutineItem;
  typedef std::vector< std::pair<RoutineHandler, size_t> > RoutineItems;
  typedef std::pair<int, ProtocolRead*> Response;
  typedef std::vector<Response> Responses;
  typedef std::vector<int> Errors;
   
 public:
  Magneto();

  bool Init(
      const std::string& conf_service_dir,
      const ReqHandler& req_handler,
      const RoutineItems* routine_items,
      void* args,
      bool& end);

  int Write(
      IN const std::string& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Errors& errors);

  int Read(
      IN const std::string& services_set,
      IN time_t timeo_ms,
      OUT Responses& results);

  int Talks(
      IN const std::string& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Responses& results);

  int Write(
      const std::string& service, 
      const Buf& buf, 
      time_t timeo_ms); 

  int Read(
      IN const std::string& service, 
      IN time_t timeo_ms, 
      OUT ProtocolRead*& protocol); 

  int SimpleTalk(
      IN const std::string& service,
      IN const Buf& buf,
      IN time_t timeo_ms,
      OUT ProtocolRead*& protocol);

  int WriteBack(const Buf& buf, time_t timeo_ms);

  void FreeTalks();

  void Stop();

  virtual ~Magneto();
 
 private:
  MagnetoBase* magneto_base_; 
};

}
