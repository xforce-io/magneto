#pragma once

#include "../common.h"
#include "../lock_policy.hpp"

namespace xforce {

template <
  typename Obj, 
  typename LockPolicy>
class PoolObjsBase {
 public:
  static const size_t kDefaultInitNumObjs=0;
  static const size_t kDefaultMemPerBlock=(1<<16);
  static const bool kDefaultToResize=true;
  static const bool kDefaultNewIfNoResize=true;
  static const size_t kDefaultMaxNumBlocks=(1<<18);

 public:
  explicit PoolObjsBase() {}
  explicit PoolObjsBase(
      const Obj& obj,
      size_t init_num_objs=kDefaultInitNumObjs,
      size_t mem_per_block=kDefaultMemPerBlock,
      bool to_resize=kDefaultToResize,
      bool new_if_no_resize=kDefaultNewIfNoResize,
      size_t max_num_blocks=kDefaultMaxNumBlocks);

  Obj* Get();
  inline void Free(Obj* obj);
  inline size_t NumObjsLeft() const;
  inline size_t NumObjsAlloc() const;
  size_t InitNumObjs() const { return init_num_objs_; }
  size_t NumObjsPerBlock() const { return num_objs_per_block_; }
  virtual size_t MemCost() const;
  virtual void DumpMemCost(std::ostream& os) const;

  virtual ~PoolObjsBase();

 protected:
  virtual void InitObj_(Obj&) {}

 private:
  bool Init_(); 
  bool AllocABlock_();

 protected:
  ///const
  Obj init_obj_; 

 private:
  ///const 
  size_t init_num_objs_;
  size_t num_objs_per_block_;
  bool to_resize_;
  bool new_if_no_resize_;
  size_t max_num_blocks_;
  LockPolicy lock_policy_;
  //

  Obj*** pool_objs_;
  size_t current_row_;
  size_t current_col_;
  size_t num_objs_alloc_;
  size_t num_blocks_alloc_;

  bool init_;
};

template <typename Obj, typename LockPolicy=NoLock>
class PoolObjs : public PoolObjsBase<Obj, LockPolicy> {
 private:
  typedef PoolObjsBase<Obj, LockPolicy> Super;
  typedef PoolObjs<Obj, LockPolicy> Self;

 public:
  explicit PoolObjs(
      size_t init_num_objs=Super::kDefaultInitNumObjs,
      size_t mem_per_block=Super::kDefaultMemPerBlock,
      bool to_resize=Super::kDefaultToResize,
      bool new_if_no_resize=Super::kDefaultNewIfNoResize,
      size_t max_num_blocks=Super::kDefaultMaxNumBlocks) :
    Super(Obj(), init_num_objs, mem_per_block, to_resize, new_if_no_resize, max_num_blocks) {}

  size_t MemCost() const;
  void DumpMemCost(std::ostream& os) const;
};

template <
  typename Obj, 
  typename LockPolicy=NoLock>
class PoolObjsInit : public PoolObjsBase<Obj, LockPolicy> {
 private:
  typedef PoolObjsBase<Obj, LockPolicy> Super;
  typedef PoolObjsInit<Obj, LockPolicy> Self;
  
 public:
  explicit PoolObjsInit() {}
  explicit PoolObjsInit(
      const Obj& obj,
      size_t init_num_objs=Super::kDefaultInitNumObjs,
      size_t mem_per_block=Super::kDefaultMemPerBlock,
      bool to_resize=Super::kDefaultToResize,
      bool new_if_no_resize=Super::kDefaultNewIfNoResize,
      size_t max_num_blocks=Super::kDefaultMaxNumBlocks):
    Super(obj, init_num_objs, mem_per_block, to_resize, new_if_no_resize, max_num_blocks) {}  

  size_t MemCost() const;
  void DumpMemCost(std::ostream& os) const;

 private:
  void InitObj_(Obj& obj) { obj = Super::init_obj_; }
};

template <typename Obj, typename LockPolicy>
PoolObjsBase<Obj, LockPolicy>::PoolObjsBase(
    const Obj& obj,
    size_t init_num_objs,
    size_t mem_per_block,
    bool to_resize,
    bool new_if_no_resize,
    size_t max_num_blocks) :
  init_obj_(obj),
  init_num_objs_(init_num_objs),
  num_objs_per_block_((mem_per_block + sizeof(Obj)-1)/sizeof(Obj)),
  to_resize_(to_resize),
  new_if_no_resize_(new_if_no_resize),
  max_num_blocks_(max_num_blocks),
  pool_objs_(NULL),
  current_row_(0),
  current_col_(0),
  num_objs_alloc_(0),
  num_blocks_alloc_(0),
  init_(false) { }

template <typename Obj, typename LockPolicy>
bool PoolObjsBase<Obj, LockPolicy>::Init_() {
  XFC_FAIL_HANDLE(0==max_num_blocks_)

  XFC_NEW(pool_objs_, Obj** [max_num_blocks_])

  bzero(pool_objs_, max_num_blocks_*sizeof(Obj**));
  while (num_blocks_alloc_*num_objs_per_block_ < init_num_objs_) {
    bool ret = AllocABlock_();
    XFC_FAIL_HANDLE(true!=ret)
  }
  init_=true;
  return true;

  ERROR_HANDLE:
  XFC_DELETE_ARRAY(pool_objs_)
  return false;
}

template <typename Obj, typename LockPolicy>
bool PoolObjsBase<Obj, LockPolicy>::AllocABlock_() {
  size_t i=0;
  
  if ( unlikely(max_num_blocks_==num_blocks_alloc_) ) return false;

  XFC_NEW(pool_objs_[num_blocks_alloc_], Obj* [num_objs_per_block_])

  for (i=0; i<num_objs_per_block_; ++i) {
    XFC_NEW(pool_objs_[current_row_][i], Obj)
    InitObj_(*(pool_objs_[current_row_][i]));
    ++num_objs_alloc_;
  }

  ++num_blocks_alloc_;
  ++current_row_;
  return true;
}

template <typename Obj, typename LockPolicy>
Obj* PoolObjsBase<Obj, LockPolicy>::Get() {
  XFC_RAII_INIT(NULL)

  if ( unlikely(!lock_policy_.Lock()) ) return NULL;

  Obj* ret;
  if (0!=current_col_) {
    ret = pool_objs_[current_row_][--current_col_];
  } else if (0!=current_row_) {
    current_col_ = num_objs_per_block_-1;
    ret = pool_objs_[--current_row_][current_col_];
  } else {
    if ( to_resize_ && AllocABlock_() ) {
      current_row_=0;
      current_col_ = num_objs_per_block_-1;
      ret = pool_objs_[current_row_][current_col_];
    } else if (new_if_no_resize_) {
      XFC_NEW(ret, Obj)
      InitObj_(*ret);
    } else {
      ret=NULL;
    }
  }
  lock_policy_.Unlock();
  return ret;
}

template <typename Obj, typename LockPolicy>
void PoolObjsBase<Obj, LockPolicy>::Free(Obj* obj) {
  XFC_RAII_INIT()
  if ( unlikely(NULL==obj || !lock_policy_.Lock()) ) {
    return;
  } else if (!to_resize_) {
    if (NumObjsLeft() == init_num_objs_) {
      delete obj;
      lock_policy_.Unlock();
      return;
    }
  }

  if ( unlikely(num_blocks_alloc_==current_row_) ) {
    if ( unlikely(max_num_blocks_==num_blocks_alloc_) ) {
      lock_policy_.Unlock();
      return;
    }

    XFC_NEW(pool_objs_[num_blocks_alloc_], Obj* [num_objs_per_block_])

    ++num_blocks_alloc_;
  }

  if (num_objs_per_block_-1 != current_col_) {
    pool_objs_[current_row_][current_col_++] = obj;
  } else {
    pool_objs_[current_row_++][current_col_] = obj;
    current_col_=0;
  }
  lock_policy_.Unlock();
}

template <typename Obj, typename LockPolicy>
size_t PoolObjsBase<Obj, LockPolicy>::NumObjsLeft() const {
  return current_row_*num_blocks_alloc_ + current_col_;
} 

template <typename Obj, typename LockPolicy>
size_t PoolObjsBase<Obj, LockPolicy>::NumObjsAlloc() const {
  return num_objs_alloc_ - NumObjsLeft();
} 

template <typename Obj, typename LockPolicy>
size_t PoolObjsBase<Obj, LockPolicy>::MemCost() const {
  return num_objs_alloc_ * sizeof(Obj) +
    num_blocks_alloc_ * num_objs_per_block_ * sizeof(Obj*) +
    sizeof(Obj**) * max_num_blocks_;
}

template <typename Obj, typename LockPolicy>
void PoolObjsBase<Obj, LockPolicy>::DumpMemCost(std::ostream& os) const {
  os << "block buckets cost: " << sizeof(Obj**) * max_num_blocks_ << std::endl;
  os << "blocks cost: " 
      << num_blocks_alloc_ * num_objs_per_block_ * sizeof(Obj*) 
      << std::endl;

  os << "objs cost: " << num_objs_alloc_ * sizeof(Obj) << std::endl;
}

template <typename Obj, typename LockPolicy>
PoolObjsBase<Obj, LockPolicy>::~PoolObjsBase() {
  if (true != lock_policy_.Lock()) return;

  if (NULL!=pool_objs_) {
    for (size_t i=0; i<current_row_; ++i) {
      for (size_t j=0; j<num_objs_per_block_; ++j) {
        XFC_DELETE(pool_objs_[i][j]);
      }
      XFC_DELETE_ARRAY(pool_objs_[i])
    }

    if (current_row_!=num_blocks_alloc_) {
      for (size_t i=0; i<current_col_; ++i) {
        XFC_DELETE(pool_objs_[current_row_][i]);
      }
      XFC_DELETE_ARRAY(pool_objs_[current_row_])

      for (size_t i=current_row_; i<num_blocks_alloc_; ++i) {
        XFC_DELETE_ARRAY(pool_objs_[i])
      }
    }
    XFC_DELETE_ARRAY(pool_objs_)
  }
  lock_policy_.Unlock();
}

template <typename Obj, typename LockPolicy>
size_t PoolObjs<Obj, LockPolicy>::MemCost() const {
  return Super::MemCost() + sizeof(Self);
}

template <typename Obj, typename LockPolicy>
void PoolObjs<Obj, LockPolicy>::DumpMemCost(std::ostream& os) const {
  os << "Self cost: " << sizeof(Self) << std::endl;
}

template <typename Obj, typename LockPolicy>
size_t PoolObjsInit<Obj, LockPolicy>::MemCost() const {
  return Super::MemCost() + sizeof(Self);
}

template <typename Obj, typename LockPolicy>
void PoolObjsInit<Obj, LockPolicy>::DumpMemCost(std::ostream& os) const {
  os << "Self cost: " << sizeof(Self) << std::endl;
}

}
