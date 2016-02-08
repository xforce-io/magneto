#pragma once

#include "../protocol.h"
#include "../../../deps/public-cpp/include/public-cpp/buffer.hpp"

namespace xforce { namespace magneto {

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

  inline void Reset(const Buf& buf);
  inline bool Encode();
  int Write(int fd);

  virtual ~ProtocolWriteRapid() {}

 private:
  Slice buf_;

  RapidHeader header_;

  iovec tmp_iovs_[2];
  size_t tmp_num_iovs_;
};

class ProtocolReadRapid : public ProtocolReadWithFixSize<RapidHeader> {
 private:
  typedef ProtocolReadWithFixSize<RapidHeader> Super;
  
 public:
  static const Protocol::Category kCategory = Protocol::kRapid;
  
 public:
  Protocol::Category GetCategory() const { return kCategory; }

  inline void Reset(const ListenAddr* listen_addr);
  inline bool Decode();

  size_t SizeBody() const { return ntohl(Header().size); }

  virtual ~ProtocolReadRapid() {}
};

void ProtocolWriteRapid::Reset(const Buf& buf) {
  buf_ = buf.first;

  tmp_iovs_[0].iov_base = RCAST<char*>(&header_);
  tmp_iovs_[0].iov_len = sizeof(header_);
  tmp_iovs_[1].iov_base = CCAST<char*>(buf_.Data());
  tmp_iovs_[1].iov_len = buf_.Size();
  tmp_num_iovs_=2;
}

bool ProtocolWriteRapid::Encode() {
  header_.magic = htons(RapidHeader::kMagic);
  header_.version = htons(0);
  header_.size = htonl(buf_.Size());
  return true;
}

void ProtocolReadRapid::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  buffer_.Clear();
  buffer_.Reserve(sizeof(RapidHeader));
  read_header_=true;
}

bool ProtocolReadRapid::Decode() { 
  RapidHeader& header = CCAST<RapidHeader&>(Header());
  header.magic = ntohs(header.magic);
  header.version = ntohs(header.version);
  if ( RapidHeader::kMagic == header.magic 
      && header.version <= RapidHeader::kMaxVersion ) {
    return true;
  } else {
    WARN("fail_decode_rapid_req magic["
        << SCAST<uint32_t>(header.magic) 
        << "] version[" 
        << SCAST<uint32_t>(header.version) 
        << "]");
    return false;
  }
}

}}
