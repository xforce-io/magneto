#pragma once

#include <memory>
#include "common.h"
#include "pool_objs/pool_objs.hpp"
#include "thread_privacy/thread_privacy.h"

namespace xforce {

template <typename Father>
class ThreadPrivatePool {
 private:
  typedef PoolObjs<Father> Pool;
  
 public:
  explicit ThreadPrivatePool();

  template <typename Son>
  inline Son* Get();

  inline void Free(Father& son);

  inline size_t NumObjs() const { return num_objs_; }

 private:
  ThreadPrivacy pools_;
  size_t num_objs_;
};

template <typename Father>
ThreadPrivatePool<Father>::ThreadPrivatePool() :
  num_objs_(0) {}

template <typename Father>
template <typename Son>
Son* ThreadPrivatePool<Father>::Get() {
  ++num_objs_;

  Pool* pool = RCAST<Pool*>(pools_.Get<Pool>(Son::kCategory));

  Father* son = pool->Get();
  if (likely( Son::kCategory == son->GetCategory() )) {
    return RCAST<Son*>(son);
  } else {
    delete son;
  }

  XFC_NEW(son, Son)
  return SCAST<Son*>(son);
}

template <typename Father>
void ThreadPrivatePool<Father>::Free(Father& son) {
  --num_objs_;

  Pool* pool = RCAST<Pool*>(pools_.Get<Pool>(son.GetCategory()));
  pool->Free(&son);
}

}
