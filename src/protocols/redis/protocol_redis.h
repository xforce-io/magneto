#pragma once

#include "../protocol.h"

namespace xforce { namespace magneto {

class ProtocolWriteRedis : public ProtocolWrite {
 public:
  static const Protocol::Category kCategory = Protocol::kRedis;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const Buf& buf);
  bool Encode();
  inline int Write(int fd);

  virtual ~ProtocolWriteRedis() {}

 private:
  std::string query_;
  std::string out_;
  size_t out_pos_;
};

class ProtocolReadRedis : public ProtocolRead {
 private:
  typedef ProtocolRead Super;
  
 public:
  static const Protocol::Category kCategory = Protocol::kRedis;
  static const int kTmpBufSize=100;
  
 public:
  ProtocolReadRedis();
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const ListenAddr* listen_addr);
  int Read(int fd);
  bool Decode() { return true; }

  const char* Data() const { return reply_.c_str(); }
  size_t Size() const { return reply_.length(); }

  virtual ~ProtocolReadRedis();

 private:
  std::string in_;
  std::string reply_;
  char* tmpbuf_;
};

void ProtocolWriteRedis::Reset(const Buf& buf) {
  query_.assign(buf.first.Data());
  out_pos_=0;
}

void ProtocolReadRedis::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  in_.clear();
  reply_.clear();
}

}}
