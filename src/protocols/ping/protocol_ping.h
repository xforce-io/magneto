#pragma once

#include "../protocol.h"

namespace magneto {

class ProtocolWritePing : public ProtocolWrite {
 public:
  static const Protocol::Category kCategory = Protocol::kPing;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  void Reset(const char* /*buf*/, size_t /*size*/) {}
  bool Encode() { return true; }
  int Write(int fd);

  virtual ~ProtocolWritePing() {}

 private:
  const char* buf_;
  size_t size_;
  size_t pos_;
};

class ProtocolReadPing : public ProtocolRead {
 private:
  typedef ProtocolRead Super;
  
 public:
  static const Protocol::Category kCategory = Protocol::kPing;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const ListenAddr* listen_addr);
  bool Decode() { return true; }
  int Read(int fd);

  const char* Buf() const { return &buffer_; }
  size_t Len() const { return sizeof(buffer_); }

  virtual ~ProtocolReadPing() {}

 private:
  char buffer_; 
};

void ProtocolReadPing::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);
}

}
