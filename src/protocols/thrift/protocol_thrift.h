#pragma once

#include "../protocol.h"
#include "../../../deps/public-cpp/include/public-cpp/buffer.hpp"
#include "t_simple_protocol.h"

#ifdef MAGNETO_THRIFT_SUPPORT

namespace xforce { namespace magneto {

/*
 * TFramedTransport + TBinaryProtocol
 */
class ProtocolWriteThrift : public ProtocolWrite {
 public:
  enum Direction {
    kRequest,
    kReply,
    kOneway,
  };
 
  struct Params {
    inline Params(const std::string& fn, Direction dir, int seqid);

    std::string fn;
    Direction dir;
    int seqid;
  };

 public:
  static const Protocol::Category kCategory = Protocol::kThrift;
 
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const Buf& buf);
  inline bool Encode();
  int Write(int fd);

  virtual ~ProtocolWriteThrift() {}

 private:
  TSimpleProtocol protocol_;
  
  const Params* params_;

  Slice struct_;
  uint32_t size_all_;

  iovec tmp_iovs_[3];
  size_t tmp_num_iovs_;
};

class ProtocolReadThrift : public ProtocolReadWithFixSize<uint32_t> {
 private:
  typedef ProtocolRead Super;

 public:
  enum Direction {
    kRequest,
    kReply,
    kOneway,
  };
  
 public:
  static const Protocol::Category kCategory = Protocol::kThrift;
  static const uint32_t kMinSizeBuf = 9;
  static const uint32_t kMaxSizeBuf = (8<<20);
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const ListenAddr* /*listen_addr*/);
  bool Decode();
  size_t SizeBody() const { return ntohl((const uint32_t&)Header()); }

  const char* Data() const { return struct_.Data(); }
  size_t Size() const { return struct_.Size(); }

  const std::string& Fn() const { return fn_; }
  Direction Dir() const { return dir_; }
  int SeqId() const { return seqid_; }

  virtual ~ProtocolReadThrift() {}

 private:
  TSimpleProtocol protocol_;
  Slice struct_;

  std::string fn_;
  Direction dir_;
  int seqid_;
};

ProtocolWriteThrift::Params::Params(
    const std::string& fn_arg, 
    Direction dir_arg, 
    int seqid_arg) :
  fn(fn_arg),
  dir(dir_arg),
  seqid(seqid_arg) {}

void ProtocolReadThrift::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  buffer_.Clear();
  buffer_.Reserve(sizeof(uint32_t));
  read_header_=true;
}
  
}}

#endif
