#pragma once

#include "common.h"

namespace xforce {

class Slice {
 public:
  inline Slice();
  inline Slice(const char* data, size_t size);
  inline void Set(const char* data, size_t size);
  const char* Data() const { return data_; }
  size_t Size() const { return size_; }
   
 private:
  const char* data_;
  size_t size_;
};

Slice::Slice() :
  data_(NULL),
  size_(0) {}

Slice::Slice(const char* data, size_t size) :
  data_(data),
  size_(size) {}

void Slice::Set(const char* data, size_t size) {
  data_=data;
  size_=size;
}

}
