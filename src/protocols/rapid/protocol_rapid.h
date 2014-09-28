#pragma once

#include "../protocol.h"
#include "../../public/buffer.hpp"

namespace magneto {

struct RapidHeader {
  static const uint16_t kMagic=58764;
  static const uint8_t kMaxVersion=10;

  uint16_t magic;
  uint16_t version;
  uint32_t size;
};

class ProtocolWriteRapid : public ProtocolWrite {
 public:
  static const Protocol::Category kCategory = Protocol::kRapid;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const char* buf, size_t size);
  inline bool Encode();
  int Write(int fd);

  virtual ~ProtocolWriteRapid() {}

 private:
  const char* buf_;
  size_t size_;
  bool write_header_;
  RapidHeader header_;
  iovec tmp_iov_[2];
};

class ProtocolReadRapid : public ProtocolRead {
 private:
  typedef ProtocolRead Super;
  
 public:
  static const Protocol::Category kCategory = Protocol::kRapid;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const ListenAddr* listen_addr);
  int Read(int fd);
  inline bool Decode();

  inline const RapidHeader* Header() const;
  inline const char* Buf() const;

  virtual ~ProtocolReadRapid() {}

 private:
  Buffer buffer_;
  bool read_header_;
};

void ProtocolWriteRapid::Reset(const char* buf, size_t size) {
  buf_=buf;
  size_=size;

  write_header_=true;
  tmp_iov_[0].iov_base = RCAST<char*>(&header_);
  tmp_iov_[0].iov_len = sizeof(header_);
  tmp_iov_[1].iov_base = CCAST<char*>(buf_);
  tmp_iov_[1].iov_len = size_;
}

bool ProtocolWriteRapid::Encode() {
  header_.size = htonl(size_);
  header_.magic = htons(RapidHeader::kMagic);
  header_.version = htons(0);
  return true;
}

void ProtocolReadRapid::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  buffer_.Clear();
  buffer_.Reserve(sizeof(RapidHeader));
  read_header_=true;
}

bool ProtocolReadRapid::Decode() { 
  RapidHeader* header = CCAST<RapidHeader*>(Header());
  header->magic = ntohs(header->magic);
  header->version = ntohs(header->version);

  if ( RapidHeader::kMagic == header->magic 
      && header->version <= RapidHeader::kMaxVersion ) {
    return true;
  } else {
    WARN("fail_decode_rapid_req magic["
        << SCAST<uint32_t>(header->magic) 
        << "] version[" 
        << SCAST<uint32_t>(header->version) 
        << "]");
    return false;
  }
}

const RapidHeader* ProtocolReadRapid::Header() const { 
  return RCAST<const RapidHeader*>(buffer_.Start()); 
}

const char* ProtocolReadRapid::Buf() const { 
  return buffer_.Start() + sizeof(RapidHeader); 
}

}
