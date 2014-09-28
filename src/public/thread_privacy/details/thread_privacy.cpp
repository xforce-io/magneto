#include "../thread_privacy.h"

namespace magneto {

ThreadMutex ThreadPrivacy::lock_;

bool ThreadPrivacy::Init_() {
  int ret = lock_.Lock();
  if (true!=ret) return false;

  ret = pthread_key_create(&key_, NULL);
  if (0!=ret) {
    lock_.Unlock();
    return false;
  }

  init_=true;

  lock_.Unlock();
  return true;
}

ThreadPrivacy::~ThreadPrivacy() {
  if (true!=init_) return;

  for (size_t i=0; i<privacies_set_.size(); ++i) {
    Privacies::const_iterator iter;
    for (iter = privacies_set_[i]->begin(); iter != privacies_set_[i]->end(); ++iter) {
      if (NULL != iter->first) (iter->second)(iter->first);
    }
    delete privacies_set_[i];
  }
  pthread_key_delete(key_);
}

}
