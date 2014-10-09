#pragma once

#include "public.h"
#include "../model.h"

namespace magneto {

class IOBasic;
class Confs;
class Agents;
class Scheduler;
class MsgNewConn;
class MsgSession;
class MsgDestruct;
class BizProcedure;

class IOBasicTPD {
 public:
  IOBasicTPD(const Confs& confs, Agents& agents);

  int Write(
      IN BizProcedure& biz_procedure,
      IN const ServicesSet& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Errors& errors);

  int Read(
      IN BizProcedure& biz_procedure,
      IN const ServicesSet& services_set,
      IN time_t timeo_ms,
      OUT Responses& responses);

  int Talks(
      IN BizProcedure& biz_procedure,
      IN const ServicesSet& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Responses& responses);

  int Write(
      BizProcedure& biz_procedure, 
      const Service& service,
      const Buf& buf,
      time_t timeo_ms); 

  int Read(
      IN BizProcedure& biz_procedure, 
      IN const Service& service, 
      IN time_t timeo_ms, 
      OUT ProtocolRead*& protocol_read); 

  int SimpleTalk(
      IN BizProcedure& biz_procedure,
      IN const Service& service,
      IN const Buf& buf,
      IN time_t timeo_ms,
      OUT ProtocolRead*& protocol_read);

  int WriteBack(BizProcedure& biz_procedure, const Buf& buf, time_t timeo_ms);

  inline void FreeTalks(BizProcedure& biz_procedure);

  virtual ~IOBasicTPD();

 private: 
  bool Init_();
  std::vector<IOBasic>* BuildIOBasics_();

 private:
  bool init_;

  const Confs* confs_;
  Agents* agents_; 

  MsgSession* tmp_msg_session_;
};

class IOBasic {
 public:
  IOBasic();

  bool Init(const Confs& confs, Agents& agents);

  inline int Write(
      IN BizProcedure& biz_procedure,
      IN const ServicesSet& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Errors& errors);

  inline int Read(
      IN BizProcedure& biz_procedure,
      IN const ServicesSet& services_set,
      IN time_t timeo_ms,
      OUT Responses& responses);

  inline int Talks(
      IN BizProcedure& biz_procedure,
      IN const ServicesSet& services_set,
      IN const Bufs& bufs,
      IN time_t timeo_ms,
      OUT Responses& responses);

  inline int Write(
      BizProcedure& biz_procedure,
      const Service& service, 
      const Buf& buf, 
      time_t timeo_ms);

  inline int Read(
      IN BizProcedure& biz_procedure,
      IN const Service& service, 
      IN time_t timeo_ms, 
      OUT ProtocolRead*& protocol_read); 

  inline int SimpleTalk(
      IN BizProcedure& biz_procedure,
      IN const Service& service,
      IN const Buf& buf,
      IN time_t timeo_ms,
      OUT ProtocolRead*& protocol_read);

  inline int WriteBack(BizProcedure& biz_procedure, const Buf& buf, time_t timeo_ms);

  inline void FreeTalks(BizProcedure& biz_procedure);

  virtual ~IOBasic();
 
 private:
  const Confs* confs_;
  Agents* agents_;
  ThreadPrivacy thread_privacy_;
  IOBasicTPD* tmp_io_basic_;
};

}

#include "../biz_procedure.h"

namespace magneto {

void IOBasicTPD::FreeTalks(BizProcedure& biz_procedure) {
  return biz_procedure.FreeTalks();
}

int IOBasic::Write(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set, 
    const Bufs& bufs, 
    time_t timeo_ms,
    Errors& errors) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->Write(biz_procedure, services_set, bufs, timeo_ms, errors);
}

int IOBasic::Read(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set, 
    time_t timeo_ms, 
    Responses& responses) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->Read(biz_procedure, services_set, timeo_ms, responses);
}

int IOBasic::Talks(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    Responses& responses) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->Talks(biz_procedure, services_set, bufs, timeo_ms, responses);
}

int IOBasic::Write(
    BizProcedure& biz_procedure, 
    const Service& service, 
    const Buf& buf, 
    time_t timeo_ms) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->Write(biz_procedure, service, buf, timeo_ms);
}

int IOBasic::Read(
    BizProcedure& biz_procedure, 
    const Service& service, 
    time_t timeo_ms, 
    ProtocolRead*& protocol_read) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->Read(biz_procedure, service, timeo_ms, protocol_read);
}

int IOBasic::SimpleTalk(
    BizProcedure& biz_procedure,
    const Service& service,
    const Buf& buf,
    time_t timeo_ms,
    ProtocolRead*& protocol_read) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->SimpleTalk(biz_procedure, service, buf, timeo_ms, protocol_read);
}

int IOBasic::WriteBack(BizProcedure& biz_procedure, const Buf& buf, time_t timeo_ms) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->WriteBack(biz_procedure, buf, timeo_ms);
}

void IOBasic::FreeTalks(BizProcedure& biz_procedure) {
  IOBasicTPD* io_basic = thread_privacy_.Get<IOBasicTPD>(0, *tmp_io_basic_);
  return io_basic->FreeTalks(biz_procedure);
}

}
