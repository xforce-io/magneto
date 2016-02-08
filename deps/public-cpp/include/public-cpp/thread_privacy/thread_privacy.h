#pragma once

#include "../common.h"
#include "../lock_policy.hpp"

namespace xforce {

class ThreadPrivacy {
 public:
  typedef void (*Deletor)(void*);
  typedef std::vector< std::pair<void*, Deletor> > Privacies;
 
 private:
  static const size_t kMaxTimesTryLock=1000000; 

 public:
  explicit ThreadPrivacy() : init_(false) {}

  inline void* Get(size_t no=0) const;

  template <typename Privacy>
  Privacy* Get(
      size_t no,
      const Privacy& privacy,
      bool* first_create=NULL);

  template <typename Privacy>
  inline Privacy* Get(size_t no=0, bool* first_create=NULL);

  virtual ~ThreadPrivacy();

 private:
  bool Init_();

  template <typename Privacy>
  int Get_(IN size_t no, OUT void*& thread_privacy, OUT Privacies*& privacies);

  template <typename Privacy>
  inline static void Delete_(void* privacy);

 private:
  pthread_key_t key_;
  std::vector<Privacies*> privacies_set_;
  static ThreadMutex lock_;

  bool init_;
};

void* ThreadPrivacy::Get(size_t no) const {
  if (unlikely(true!=init_)) return NULL;

  Privacies* privacies = RCAST<Privacies*>(pthread_getspecific(key_));
  if ( unlikely(NULL==privacies) ) return NULL;

  if (no < privacies->size() && NULL != (*privacies)[no].first) {
    return (*privacies)[no].first;
  } else {
    return NULL;
  }
}

template <typename Privacy>
Privacy* ThreadPrivacy::Get(
    size_t no, 
    const Privacy& privacy_tpl,
    bool* first_create) {
  XFC_RAII_INIT(NULL)

  void* result;
  Privacies* privacies;
  Privacy* privacy=NULL;
  int ret;

  ret = Get_<Privacy>(no, result, privacies);
  if (0==ret) {
    if (NULL!=first_create) *first_create=false;
    return RCAST<Privacy*>(result);
  } else if(ret<0) {
    XFC_FAIL_HANDLE(true)
  }

  privacy = new (std::nothrow) Privacy(privacy_tpl);
  XFC_FAIL_HANDLE(NULL==privacy)

  if (no >= privacies->size()) {
    size_t old_size = privacies->size();
    privacies->resize(no+1);
    for (size_t i=old_size; i<no; ++i) (*privacies)[i].first = NULL;
  }
  (*privacies)[no] = std::pair<void*, Deletor>(privacy, Delete_<Privacy>);

  if (NULL!=first_create) *first_create=true;
  return privacy;

  ERROR_HANDLE:
  if (NULL!=privacy) delete privacy;
  return NULL;
}

template <typename Privacy>
Privacy* ThreadPrivacy::Get(size_t no, bool* first_create) { 
  XFC_RAII_INIT(NULL)

  void* result=NULL;
  Privacies* privacies;
  Privacy* privacy=NULL;
  int ret;

  ret = Get_<Privacy>(no, result, privacies);
  if (0==ret) {
    if (NULL!=first_create) *first_create=false;
    return RCAST<Privacy*>(result);
  } else if(ret<0) {
    XFC_FAIL_HANDLE(true)
  }

  privacy = new (std::nothrow) Privacy;
  XFC_FAIL_HANDLE(NULL==privacy)

  if (no >= privacies->size()) {
    size_t old_size = privacies->size();
    privacies->resize(no+1);
    for (size_t i=old_size; i<no; ++i) (*privacies)[i].first = NULL;
  }
  (*privacies)[no] = std::pair<void*, Deletor>(privacy, Delete_<Privacy>);

  if (NULL!=first_create) *first_create=true;
  return privacy;

  ERROR_HANDLE:
  if (NULL!=privacy) delete privacy;
  return NULL;
}

template <typename Privacy>
int ThreadPrivacy::Get_(size_t no, void*& thread_privacy, Privacies*& privacies) {
  Privacies::iterator iter;
  privacies = (Privacies*)pthread_getspecific(key_);
  if (privacies) {
    if (no < privacies->size() && (*privacies)[no].first) {
      thread_privacy = (*privacies)[no].first;
      return 0;
    }
  } else {
    privacies = new (std::nothrow) Privacies;
    if (NULL==privacies) return -1;

    int ret = pthread_setspecific(key_, privacies);
    if (0!=ret) {
      delete privacies; 
      return -2;
    }

    for (size_t i=0; i<kMaxTimesTryLock; ++i) {
      ret = lock_.Lock();
      if (true==ret) break;
    }

    if (false==ret) return -3;

    privacies_set_.push_back(privacies);
    lock_.Unlock();
  }
  return 1;
}

template <typename Privacy>
void ThreadPrivacy::Delete_(void* privacy) {
  delete RCAST<Privacy*>(privacy);
}

}
