#pragma once 

#include "public.h"

namespace xforce { namespace magneto {

class CtxHelper {
 private:
  static const size_t kLow32Int = ((size_t(1) << 32) - 1);

 public:
  template <typename RetType>
  inline static void PtrToInts(RetType* args, int& hign, int& low); 

  template <typename RetType>
  inline static RetType* IntsToPtr(int hign, int low);
};

template <typename RetType>
void CtxHelper::PtrToInts(RetType* args, int& hign, int& low) {
  hign = (int)( RCAST<size_t>(args) >> 32 );
  low = (int)( RCAST<size_t>(args) & kLow32Int );
}

template <typename RetType>
RetType* CtxHelper::IntsToPtr(int hign, int low) {
  size_t hign_ptr = (uint32_t)hign;
  size_t low_ptr = (uint32_t)low;
  return RCAST<RetType*>( (hign_ptr<<32) | low_ptr );
}

}}
