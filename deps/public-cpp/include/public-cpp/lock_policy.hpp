#pragma once

#include "common.h"

namespace xforce {

class NoLock {
 public:
  bool Lock() const { return true; }
  void Unlock() const {}
};

/* RAII */
class ThreadMutex {
 public:
  explicit ThreadMutex() : init_(false) {}

  inline bool Lock();
  inline void Unlock();

 private:
  inline bool Init_();

 private:
  pthread_mutex_t mutex_;

  bool init_;
};

class SpinLock {
 public:
  inline SpinLock(); 
  inline bool Lock();
  inline void Unlock();
  inline virtual ~SpinLock();

 private:
  inline bool Init_();

 private:
  pthread_spinlock_t lock_;

  bool init_;
};

bool ThreadMutex::Lock() { 
  XFC_RAII_INIT(false)
  return 0 == pthread_mutex_lock(&mutex_); 
}

void ThreadMutex::Unlock() { 
  XFC_RAII_INIT()
  pthread_mutex_unlock(&mutex_); 
}

bool ThreadMutex::Init_() { 
  if (0 == pthread_mutex_init(&mutex_, NULL)) {
    init_=true;
    return true;
  } else {
    return false;
  }
}

SpinLock::SpinLock() :
  init_(false) {}

bool SpinLock::Lock() {
  XFC_RAII_INIT(false)
  return 0 == pthread_spin_lock(&lock_);
}

void SpinLock::Unlock() {
  XFC_RAII_INIT()
  pthread_spin_unlock(&lock_);
}

SpinLock::~SpinLock() {
  if (true==init_) {
    pthread_spin_destroy(&lock_);
  }
}

bool SpinLock::Init_() {
  if (0 == pthread_spin_init(&lock_, 0)) {
    init_=true;
    return true;
  } else {
    return false;
  }
}

}
