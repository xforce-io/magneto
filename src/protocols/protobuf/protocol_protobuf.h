#pragma once

#include <zlib.h>

#include "../protocol.h"
#include "../../../deps/public-cpp/include/public-cpp/buffer.hpp"

#ifdef MAGNETO_PROTOBUF_SUPPORT

#include "google/protobuf/message.h"

using google::protobuf::Message;

namespace xforce { namespace magneto {

class ProtocolWriteProtobuf : public ProtocolWrite {
 public:
  static const Protocol::Category kCategory = Protocol::kProtobuf;  

 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const Buf& buf);
  bool Encode();
  int Write(int fd);

  virtual ~ProtocolWriteProtobuf() {}

 private:
  const char* msg_name_;
  const Message* msg_;

  std::string pb_data_;
  std::string out_;
  size_t out_pos_;
};

class ProtocolReadProtobuf : public ProtocolReadWithFixSize<uint32_t> {
 private:
  typedef ProtocolReadWithFixSize<uint32_t> Super;
  
 public:
  static const Protocol::Category kCategory = Protocol::kProtobuf;
  static const int kTmpBufSize=100;
   
 public:
  inline ProtocolReadProtobuf();
  virtual Protocol::Category GetCategory() const { return Protocol::kProtobuf; }
  inline void Reset(const ListenAddr* /*listen_addr*/);
  size_t SizeBody() const { return ntohl((const uint32_t&)Header()); }

  bool Decode();
  std::string GetMsgName() const { return msg_->GetTypeName(); }
  const Message* GetMsg() const { return msg_; }

  virtual ~ProtocolReadProtobuf();

 private: 
  Message* msg_;
};

void ProtocolWriteProtobuf::Reset(const Buf& buf) {
  msg_name_ = RCAST<const char*>(buf.second);
  msg_ = RCAST<const Message*>(buf.first.Data());
  out_.clear();
  out_pos_ = 0;
}

ProtocolReadProtobuf::ProtocolReadProtobuf() :
    msg_(NULL) {}

void ProtocolReadProtobuf::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  XFC_DELETE(msg_)
}

}}

#endif
