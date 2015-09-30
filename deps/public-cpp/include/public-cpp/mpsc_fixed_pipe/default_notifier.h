#pragma once

#include "../common.h"
#include "../lock_policy.hpp"

namespace xforce {

class DefaultNotifier {
 private:
  static const size_t kMaxEpollEvents=10240;

 public:
  DefaultNotifier();

  bool Init(); 

  int Notify();

  /*
   * @return:
   *  >0 : num events
   * ==0 : timeout 
   *  <0 : error
   */
  int Wait(time_t time_ms);

  ~DefaultNotifier();
 
 private:
  pthread_key_t pthread_key_;
  int fd_epoll_;
  epoll_event* epoll_events_;
  SpinLock lock_fd_epoll_;
};

}
