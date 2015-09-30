#pragma once

#include "common.h"
#include "lock_policy.hpp"

namespace xforce {

template <typename Obj>
class DelayDelPtr {
 private:
  typedef DelayDelPtr<Obj> Self;
  typedef std::tr1::unordered_map< size_t, std::pair<Obj*, time_t> > DelayDelPtrMap;
  
 public:
  static const time_t kDefaultDelayDelTimeSec=60;

 public:
  explicit DelayDelPtr(
      Obj* obj=NULL,
      bool to_delete=true,
      time_t delay_del_time_in_sec=kDefaultDelayDelTimeSec);
  
  inline const Obj* Get(size_t version=-1) const;
  inline Obj* Get(size_t version=-1);
  size_t GetLatestVersion() const { return version_; }
  const Obj& operator*() const { return *obj_; }
  Obj& operator*() { return *obj_; }
  const Obj* operator->() const { return obj_; }
  Obj* operator->() { return obj_; }
  bool IsNull() const { return NULL==obj_; }
  void Change(Obj& obj);
  virtual ~DelayDelPtr();

 private:
  //const
  time_t delay_del_time_in_sec_;
  bool to_delete_;
  ///

  Obj* obj_; 
  size_t version_;
  DelayDelPtrMap delay_del_ptr_map_;
  mutable SpinLock lock_map_;
};

template <typename Obj>
DelayDelPtr<Obj>::DelayDelPtr(
    Obj* obj,
    bool to_delete,
    time_t delay_del_time_in_sec) :
  delay_del_time_in_sec_(delay_del_time_in_sec),
  to_delete_(to_delete),
  obj_(obj),
  version_(0) {}

template <typename Obj>
void DelayDelPtr<Obj>::Change(Obj& obj) {
  time_t current_time = time(NULL);

  lock_map_.Lock();
  if (!delay_del_ptr_map_.empty()) {
    typename DelayDelPtrMap::iterator iter;
    std::vector<size_t> versions_to_delete;
    for (iter = delay_del_ptr_map_.begin(); iter != delay_del_ptr_map_.end(); ++iter) {
      time_t time_passed = current_time - iter->second.second;
      if (time_passed>delay_del_time_in_sec_) {
        versions_to_delete.push_back(iter->first);
        if (to_delete_) XFC_DELETE(iter->second.first)
      }
    }

    for (size_t i=0; i < versions_to_delete.size(); ++i) {
      delay_del_ptr_map_.erase(versions_to_delete[i]);
    }
  }

  if (NULL!=obj_) {
    delay_del_ptr_map_.insert(std::make_pair(version_, std::make_pair(obj_, current_time)));
    obj_=&obj;
    ++version_;
  } else {
    obj_=&obj;
  }
  lock_map_.Unlock();
}

template <typename Obj>
const Obj* DelayDelPtr<Obj>::Get(size_t version) const {
  size_t cur_version=version_;
  if (likely(cur_version==version || size_t(-1) == version)) {
    return obj_;
  }

  lock_map_.Lock();
  typename DelayDelPtrMap::const_iterator iter = delay_del_ptr_map_.find(version);
  const Obj* obj=NULL;
  if (delay_del_ptr_map_.end() != iter) {
    obj = iter->second.first;
  }
  lock_map_.Unlock();
  return obj;
}

template <typename Obj>
Obj* DelayDelPtr<Obj>::Get(size_t version) {
  return CCAST<Obj*>(CCAST<const Self*>(this)->Get(version));
}

template <typename Obj>
DelayDelPtr<Obj>::~DelayDelPtr() {
  lock_map_.Lock();
  typename DelayDelPtrMap::iterator iter;
  for (iter = delay_del_ptr_map_.begin(); iter != delay_del_ptr_map_.end(); ++iter) {
    if (to_delete_) XFC_DELETE(iter->second.first)
  }
  delay_del_ptr_map_.clear();
  lock_map_.Unlock();

  if (to_delete_) XFC_DELETE(obj_)
}

}
