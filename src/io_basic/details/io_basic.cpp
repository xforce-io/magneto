#include "../io_basic.h"

#include "../../confs/confs.h"
#include "../../protocols/pool_protocols.h"
#include "../../msg.h"
#include "../../agents/agents.h"
#include "../../model.h"

namespace magneto {

IOBasicTPD::IOBasicTPD(const Confs& confs, Agents& agents) :
  init_(false),
  confs_(&confs),
  agents_(&agents),
  tmp_msg_session_(NULL) {}

int IOBasicTPD::Write(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    Errors& errors) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int ret;

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    int fd;
    const Remote* remote=NULL;
    biz_procedure.GetFdFromServiceCache(*(services_set[i]), fd, remote);
    (*talks)[i].Assign(
        i,
        Talk::kWriteOnly, 
        services_set[i], 
        services_set[i]->protocol_category, 
        bufs[i], 
        timeo_ms, 
        fd, 
        remote);
  }

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  errors.reserve(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    Talk& talk = (*talks)[i];
    if (talk.fd > 0) {
      biz_procedure.InsertFdIntoServiceCache(*(talk.service), talk.fd, *(talk.remote));
    } else {
      biz_procedure.InvalidFdInServiceCache(*(talk.service));
    }

    errors[i] = talk.error;
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_writes ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::Read(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set,
    time_t timeo_ms,
    Responses& responses) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int ret;

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    int fd;
    const Remote* remote=NULL;
    biz_procedure.GetFdFromServiceCache(*(services_set[i]), fd, remote);
    (*talks)[i].Assign(
        i,
        Talk::kReadOnly, 
        services_set[i], 
        services_set[i]->protocol_category,
        NULL, 
        timeo_ms, 
        fd, 
        remote);
  }

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  responses.resize(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    Talk& talk = (*talks)[i];
    if (talk.fd > 0) {
      biz_procedure.InsertFdIntoServiceCache(*(talk.service), talk.fd, *(talk.remote));
    } else {
      biz_procedure.InvalidFdInServiceCache(*(talk.service));
    }

    responses[i].first = talk.error;
    responses[i].second = talk.protocol_read;
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_reads ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::Talks(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    Responses& responses) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int ret;

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    int fd;
    const Remote* remote=NULL;
    biz_procedure.GetFdFromServiceCache(*(services_set[i]), fd, remote);
    (*talks)[i].Assign(
        i,
        Talk::kWriteAndRead, 
        services_set[i], 
        services_set[i]->protocol_category, 
        bufs[i], 
        timeo_ms, 
        fd, 
        remote);
  }

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  responses.resize(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    Talk& talk = (*talks)[i];
    if (talk.fd > 0) {
      biz_procedure.InsertFdIntoServiceCache(*(talk.service), talk.fd, *(talk.remote));
    } else {
      biz_procedure.InvalidFdInServiceCache(*(talk.service));
    }

    responses[i].first = talk.error;
    responses[i].second = talk.protocol_read;
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_talks ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::Write(
    BizProcedure& biz_procedure, 
    const Service& service,
    const Buf& buf, 
    time_t timeo_ms) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int ret;
  int fd;
  const Remote* remote=NULL;
  biz_procedure.GetFdFromServiceCache(service, fd, remote);

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(1);
  Talk& talk = (*talks)[0];
  talk.Assign(
      0,
      Talk::kWriteOnly, 
      &service, 
      service.protocol_category, 
      &buf, 
      timeo_ms, 
      fd, 
      remote);
  MAG_FAIL_HANDLE_AND_SET(ErrorNo::kOk != (*talks)[0].error, ret = (*talks)[0].error)

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoServiceCache(service, talk.fd, *(talk.remote));
  } else {
    biz_procedure.InvalidFdInServiceCache(service);
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)

  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_write ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::Read(
    BizProcedure& biz_procedure,
    const Service& service, 
    time_t timeo_ms, 
    ProtocolRead*& protocol_read) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int fd;
  const Remote* remote=NULL;
  biz_procedure.GetFdFromServiceCache(service, fd, remote);

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(1);
  Talk& talk = (*talks)[0];
  talk.Assign(
      0,
      Talk::kReadOnly, 
      &service, 
      service.protocol_category,
      NULL, 
      timeo_ms, 
      fd, 
      remote);

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  int ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoServiceCache(service, talk.fd, *(talk.remote));
  } else {
    biz_procedure.InvalidFdInServiceCache(service);
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)

  protocol_read = talk.protocol_read;
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_read ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::SimpleTalk(
    IN BizProcedure& biz_procedure,
    IN const Service& service,
    IN const Buf& buf,
    IN time_t timeo_ms,
    OUT ProtocolRead*& protocol_read) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int ret;
  int fd;
  const Remote* remote=NULL;
  biz_procedure.GetFdFromServiceCache(service, fd, remote);

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(1);
  Talk& talk = (*talks)[0];
  talk.Assign(
      0,
      Talk::kWriteAndRead, 
      &service, 
      service.protocol_category,
      &buf, 
      timeo_ms, 
      fd, 
      remote);
  MAG_FAIL_HANDLE_AND_SET(ErrorNo::kOk != (*talks)[0].error, ret = (*talks)[0].error)

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoServiceCache(service, talk.fd, *(talk.remote));
  } else {
    biz_procedure.InvalidFdInServiceCache(service);
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)

  protocol_read = talk.protocol_read;
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_simple_talk ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::WriteBack(BizProcedure& biz_procedure, const Buf& buf, time_t timeo_ms) {
  MAG_RAII_INIT(ErrorNo::kOther)

  int ret;
  biz_procedure.SetWriteBackCalled();

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(1);
  Talk& talk = (*talks)[0];
  talk.Assign(
      0,
      Talk::kWriteOnly, 
      NULL, 
      biz_procedure.GetProtocolRead()->GetCategory(),
      &buf, 
      timeo_ms, 
      biz_procedure.GetFdClient(), 
      NULL);
  MAG_FAIL_HANDLE_AND_SET(ErrorNo::kOk != (*talks)[0].error, ret = (*talks)[0].error)

  tmp_msg_session_->BuildForSession(biz_procedure.GetCtx(), biz_procedure, *talks, timeo_ms, ErrorNo::kOk);

  ret = agents_->SendMsg(biz_procedure.GetIdProcedure(), *tmp_msg_session_);
  MAG_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  ret = swapcontext(&(biz_procedure.GetCtx()), &(biz_procedure.GetScheduler()->GetCtx()));
  MAG_BUG(0!=ret)

  if (talk.fd <= 0) {
    biz_procedure.SetFdClientInvalid();
  }

  ret = biz_procedure.GetMsgSession()->error;
  MAG_FAIL_HANDLE(ErrorNo::kOk != ret)

  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_write_back ret[" << ret << "]");
  return ret;
}

IOBasicTPD::~IOBasicTPD() {
  MAG_DELETE(tmp_msg_session_)
}

bool IOBasicTPD::Init_() {
  MAG_NEW(tmp_msg_session_, MsgSession)
  init_=true;
  return true;
}

IOBasic::IOBasic() :
  tmp_io_basic_(NULL) {}

bool IOBasic::Init(const Confs& confs, Agents& agents) {
  confs_ = &confs;
  agents_ = &agents;
  MAG_NEW(tmp_io_basic_, IOBasicTPD(confs, agents))
  return true;
}

IOBasic::~IOBasic() {
  MAG_DELETE(tmp_io_basic_)
}

}
