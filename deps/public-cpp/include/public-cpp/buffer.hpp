#pragma once 

#include "common.h"

namespace xforce {

class Buffer {
 public:
  inline Buffer();
  inline void Set(const char* buf, size_t len, size_t pos=0);
  void SetLen(size_t len) { len_=len; }
  inline void Reserve(size_t len);
  void Clear() { len_=0; }

  const char* Start() const { return buf_; }
  char* Start() { return buf_; }
  const char* Stop() const { return buf_+len_; }
  char* Stop() { return buf_+len_; }
  size_t Len() const { return len_; }
  size_t Size() const { return size_; }
  
  inline virtual ~Buffer();

 private:
  char* buf_;
  size_t len_;
  size_t size_;
};

Buffer::Buffer() :
  buf_(NULL),
  len_(0),
  size_(0) {}

void Buffer::Set(const char* buf, size_t len, size_t pos) {
  Reserve(len+pos);
  memcpy(buf_+pos, buf, len);
}

void Buffer::Reserve(size_t size) {
  if (size>size_) {
    buf_ = RCAST<char*>(realloc(buf_, size));
    size_=size;
  }
}

Buffer::~Buffer() {
  if (NULL!=buf_) free(buf_);
}

}
