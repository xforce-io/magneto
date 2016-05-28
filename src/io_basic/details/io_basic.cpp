#include "../io_basic.h"

#include "../../confs/confs.h"
#include "../../protocols/pool_protocols.h"
#include "../../msg.h"
#include "../../agents/agents.h"
#include "../../model.h"

namespace xforce { namespace magneto {

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
  XFC_RAII_INIT(ErrorNo::kOther)

  std::vector<Talk>* talks = GetTalks_(
      biz_procedure, 
      services_set, 
      Talk::kWriteOnly,
      &bufs,
      timeo_ms);

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

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
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)
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
  XFC_RAII_INIT(ErrorNo::kOther)

  Talks* talks = GetTalks_(
      biz_procedure,
      services_set,
      Talk::kReadOnly,
      NULL,
      timeo_ms);

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

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
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_reads ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::ParaTalks(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    Responses& responses) {
  XFC_RAII_INIT(ErrorNo::kOther)

  Talks* talks = GetTalks_(
      biz_procedure,
      services_set,
      Talk::kWriteAndRead,
      &bufs,
      timeo_ms);

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

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
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)
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
  XFC_RAII_INIT(ErrorNo::kOther)

  ServicesSet services_set;
  services_set.push_back(&service);
  Bufs bufs;
  bufs.push_back(&buf);

  Talks* talks = GetTalks_(
      biz_procedure,
      services_set,
      Talk::kWriteOnly,
      &bufs,
      timeo_ms);
  Talk& talk = (*talks)[0];

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = talk.error)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoServiceCache(service, talk.fd, *(talk.remote));
  } else {
    biz_procedure.InvalidFdInServiceCache(service);
  }

  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)
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
  XFC_RAII_INIT(ErrorNo::kOther)

  ServicesSet services_set;
  services_set.push_back(&service);

  Talks* talks = GetTalks_(
      biz_procedure,
      services_set,
      Talk::kReadOnly,
      NULL,
      timeo_ms);
  Talk& talk = (*talks)[0];

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = talk.error)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoServiceCache(service, talk.fd, *(talk.remote));
  } else {
    biz_procedure.InvalidFdInServiceCache(service);
  }

  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)

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
  XFC_RAII_INIT(ErrorNo::kOther)

  ServicesSet services_set;
  services_set.push_back(&service);
  Bufs bufs;
  bufs.push_back(&buf);

  Talks* talks = GetTalks_(
      biz_procedure,
      services_set,
      Talk::kWriteAndRead,
      &bufs,
      timeo_ms);
  Talk& talk = (*talks)[0];

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = talk.error)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoServiceCache(service, talk.fd, *(talk.remote));
  } else {
    biz_procedure.InvalidFdInServiceCache(service);
  }

  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)

  protocol_read = talk.protocol_read;
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_simple_talk ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::Write(
    BizProcedure& biz_procedure, 
    const Service& service,
    const Remote& remote,
    const Buf& buf,
    time_t timeo_ms) {
  XFC_RAII_INIT(ErrorNo::kOther)

  Talks* talks = GetTalkForRemote_(
      biz_procedure,
      service,
      remote,
      Talk::kWriteOnly,
      &buf,
      timeo_ms);
  Talk& talk = (*talks)[0];

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = talk.error)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoRemoteCache(remote.name, talk.fd);
  } else {
    biz_procedure.InvalidFdInRemoteCache(remote.name);
  }

  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_write ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::Read(
    BizProcedure& biz_procedure, 
    const Service& service,
    const Remote& remote, 
    time_t timeo_ms, 
    ProtocolRead*& protocol_read) {
  XFC_RAII_INIT(ErrorNo::kOther)

  Talks* talks = GetTalkForRemote_(
      biz_procedure,
      service,
      remote,
      Talk::kReadOnly,
      NULL,
      timeo_ms);
  Talk& talk = (*talks)[0];

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = talk.error)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoRemoteCache(remote.name, talk.fd);
  } else {
    biz_procedure.InvalidFdInRemoteCache(remote.name);
  }

  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)

  protocol_read = talk.protocol_read;
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_read ret[" << ret << "]");
  return ret;
}

int IOBasicTPD::SimpleTalk(
    BizProcedure& biz_procedure,
    const Service& service,
    const Remote& remote,
    const Buf& buf,
    time_t timeo_ms,
    ProtocolRead*& protocol_read) {
  XFC_RAII_INIT(ErrorNo::kOther)

  ServicesSet services_set;
  services_set.push_back(&service);
  Bufs bufs;
  bufs.push_back(&buf);

  Talks* talks = GetTalks_(
      biz_procedure,
      services_set,
      Talk::kWriteAndRead,
      &bufs,
      timeo_ms);
  Talk& talk = (*talks)[0];

  int ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = talk.error)

  if (talk.fd > 0) {
    biz_procedure.InsertFdIntoRemoteCache(remote.name, talk.fd);
  } else {
    biz_procedure.InvalidFdInRemoteCache(remote.name);
  }

  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)

  protocol_read = talk.protocol_read;
  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_simple_talk ret[" << ret << "]");
  return ret;

}

int IOBasicTPD::WriteBack(BizProcedure& biz_procedure, const Buf& buf, time_t timeo_ms) {
  XFC_RAII_INIT(ErrorNo::kOther)

  biz_procedure.SetWriteBackCalled();

  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(1);
  Talk& talk = (*talks)[0];

  int ret;
  talk.Assign(
      0,
      Talk::kWriteOnly, 
      NULL, 
      biz_procedure.GetProtocolRead()->GetCategory(),
      &buf, 
      timeo_ms, 
      biz_procedure.GetFdClient(), 
      NULL);
  XFC_FAIL_HANDLE_AND_SET(ErrorNo::kOk != (*talks)[0].error, ret = (*talks)[0].error)

  ret = SendSessionAndSwap_(biz_procedure, *talks, timeo_ms);
  XFC_FAIL_HANDLE_AND_SET(true!=ret, ret = ErrorNo::kQueueBusy)

  if (talk.fd <= 0) {
    biz_procedure.SetFdClientInvalid();
  }
  ret = biz_procedure.GetMsgSession()->error;
  XFC_FAIL_HANDLE(ErrorNo::kOk != ret)

  return ErrorNo::kOk;

  ERROR_HANDLE:
  DEBUG("fail_io_basic_write_back ret[" << ret << "]");
  return ret;
}

IOBasicTPD::~IOBasicTPD() {
  XFC_DELETE(tmp_msg_session_)
}

bool IOBasicTPD::Init_() {
  XFC_NEW(tmp_msg_session_, MsgSession)
  init_=true;
  return true;
}

IOBasic::IOBasic() :
  tmp_io_basic_(NULL) {}

bool IOBasic::Init(const Confs& confs, Agents& agents) {
  confs_ = &confs;
  agents_ = &agents;
  XFC_NEW(tmp_io_basic_, IOBasicTPD(confs, agents))
  return true;
}

IOBasic::~IOBasic() {
  XFC_DELETE(tmp_io_basic_)
}

std::vector<Talk>* IOBasicTPD::GetTalks_(
    BizProcedure& biz_procedure,
    const ServicesSet& services_set,
    Talk::Category category,
    const Bufs* bufs,
    time_t timeo_ms) {
  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(services_set.size());
  for (size_t i=0; i < services_set.size(); ++i) {
    int fd;
    const Remote* remote=NULL;
    biz_procedure.GetFdFromServiceCache(*(services_set[i]), fd, remote);
    (*talks)[i].Assign(
        i,
        category,
        services_set[i],
        services_set[i]->protocol_category,
        (*bufs)[i],
        timeo_ms,
        fd,
        remote);
  }
  return talks;
}

IOBasicTPD::Talks* IOBasicTPD::GetTalkForRemote_(
    BizProcedure& biz_procedure,
    const Service& service,
    const Remote& remote,
    Talk::Category category,
    const Buf* buf,
    time_t timeo_ms) {
  std::vector<Talk>* talks = biz_procedure.GetTalk();
  talks->resize(1);

  int fd;
  biz_procedure.GetFdFromRemoteCache(remote.name, fd);
  (*talks)[0].Assign(
      0,
      category, 
      &service, 
      service.protocol_category,
      buf, 
      timeo_ms, 
      fd, 
      &remote);
  return talks;
}

}}
