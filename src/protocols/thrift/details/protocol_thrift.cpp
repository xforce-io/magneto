#include "../protocol_thrift.h"
#include "../../public.h"

#ifdef MAGNETO_THRIFT_SUPPORT

using ::apache::thrift::protocol::TMessageType;
using ::apache::thrift::protocol::T_CALL;
using ::apache::thrift::protocol::T_REPLY;
using ::apache::thrift::protocol::T_ONEWAY;
using ::apache::thrift::protocol::T_ONEWAY;
using ::apache::thrift::TException;

namespace xforce { namespace magneto {

void ProtocolWriteThrift::Reset(const Buf& buf) {
  struct_ = buf.first;
  params_ = RCAST<const Params*>(buf.second);
}

bool ProtocolWriteThrift::Encode() {
  TMessageType mtype;
  switch (params_->dir) {
    case kRequest: mtype=T_CALL; break;
    case kReply: mtype=T_REPLY; break;
    case kOneway: mtype=T_ONEWAY; break;
    default: {
      DEBUG("invalid_dir[" << params_->dir << "]");
      return false;
    }
  }

  protocol_.GetMemBuf().resetBuffer();
  try {
    protocol_.writeMessageBegin(params_->fn, mtype, params_->seqid);
    protocol_.writeMessageEnd();
    protocol_.getTransport()->writeEnd();
    protocol_.getTransport()->flush();
  } catch (const TException& ex) {
    DEBUG("fail_encode_thrift");
    return false;
  }

  uint32_t bytes_wrote;
  uint8_t* buf_proto;
  protocol_.GetMemBuf().getBuffer(&buf_proto, &bytes_wrote);
  size_all_ = htonl(bytes_wrote + struct_.Size());

  tmp_iovs_[0].iov_base = &size_all_;
  tmp_iovs_[0].iov_len = sizeof(uint32_t);
  tmp_iovs_[1].iov_base = buf_proto;
  tmp_iovs_[1].iov_len = bytes_wrote;
  tmp_iovs_[2].iov_base = CCAST<char*>(struct_.Data());
  tmp_iovs_[2].iov_len = struct_.Size();
  tmp_num_iovs_=3;
  return true;
}

int ProtocolWriteThrift::Write(int fd) {
  int ret = IOHelper::WriteVecNonBlock(fd, tmp_iovs_, tmp_num_iovs_);
  if (ret>0) {
    return 0==tmp_num_iovs_ ? 
      0 : 
      ( 1==tmp_num_iovs_ ? 
          tmp_iovs_[0].iov_len : 
          ( 2==tmp_num_iovs_ ?
              tmp_iovs_[0].iov_len + tmp_iovs_[1].iov_len :
              tmp_iovs_[0].iov_len + tmp_iovs_[1].iov_len + tmp_iovs_[2].iov_len ) );
  } else {
    return 0==ret ? kEnd : ret;
  }
}

bool ProtocolReadThrift::Decode() {
  if (SizeBody() < kMinSizeBuf || SizeBody() > kMaxSizeBuf) {
    DEBUG("invalid_packet_len[" << SizeBody() << "]");
    return false;
  }

  protocol_.GetMemBuf().resetBuffer(RCAST<uint8_t*>(Body()), SizeBody());
  TMessageType mtype;
  try {
    uint32_t size_func = protocol_.readMessageBegin(fn_, mtype, seqid_); 
    switch (mtype) {
      case T_CALL: dir_=kRequest; break;
      case T_REPLY: dir_=kReply; break;
      case T_ONEWAY: dir_=kOneway; break;
      default :
        DEBUG("invalid_mtype[" << mtype << "]");
        return false;
    }

    struct_.Set(Body() + size_func, SizeBody() - size_func);
    protocol_.getTransport()->consume(struct_.Size());
    protocol_.readMessageEnd();
    protocol_.getTransport()->readEnd();
  } catch (const TException &ex) {
    DEBUG("fail_decode_thrift");
    return false;
  }
  return true;
}

}}

#endif
