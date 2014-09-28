#pragma once

#include "../protocol.h"

namespace magneto {

class ProtocolWriteRedis : public ProtocolWrite {
 public:
  static const Protocol::Category kCategory = Protocol::kRedis;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const char* buf, size_t size);
  bool Encode();
  inline int Write(int fd);

  virtual ~ProtocolWriteRedis() {}

 private:
  std::string in_;
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

  const char* Buf() const { return out_.data(); }
  size_t Len() const { return out_.size(); }

  virtual ~ProtocolReadRedis();

 private:
  std::string reply_;
  std::string out_;
  char* tmpbuf_;
};

void ProtocolWriteRedis::Reset(const char* buf, size_t /*size*/) {
  in_.assign(buf);
  out_pos_=0;
}

void ProtocolReadRedis::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  reply_.clear();
  out_.clear();
}

}
