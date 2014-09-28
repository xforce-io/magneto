#include "../default_notifier.h"
#include "../../io_helper.h"

namespace magneto {

DefaultNotifier::DefaultNotifier() :
  pthread_key_(0),
  fd_epoll_(0),
  epoll_events_(NULL) {}

bool DefaultNotifier::Init() {
  int ret = pthread_key_create(&pthread_key_, NULL);
  if (0!=ret) return false;

  fd_epoll_ = epoll_create(100);
  if (fd_epoll_<=0) return false;

  MAG_NEW(epoll_events_, epoll_event [kMaxEpollEvents])
  return true;
}

int DefaultNotifier::Notify() {
  int* fd = RCAST<int*>(pthread_getspecific(pthread_key_));
  if (unlikely(NULL==fd)) {
    int pipefd[2];
    int ret = pipe(pipefd);
    if (0!=ret || !IOHelper::SetNonBlock(pipefd[0]) || !IOHelper::SetNonBlock(pipefd[1])) {
      return -1;
    }

    ret = lock_fd_epoll_.Lock();
    if (true!=ret) return -2;

    epoll_event ee;
    ee.events = (EPOLLIN|EPOLLERR|EPOLLHUP);
    ee.data.fd = pipefd[0];
    ret = epoll_ctl(fd_epoll_, EPOLL_CTL_ADD, pipefd[0], &ee);
    if (0!=ret) return -3;

    lock_fd_epoll_.Unlock();

    int* send_fd = new int(pipefd[1]);
    ret = pthread_setspecific(pthread_key_, send_fd);
    if (0!=ret) {
      epoll_ctl(fd_epoll_, EPOLL_CTL_DEL, *send_fd, NULL);
      delete send_fd;
      return -4;
    }

    fd=send_fd;
  }

  char buf[1];
  ssize_t ret = write(*fd, buf, 1);
  return 1==ret ? 0 : -5;
}

int DefaultNotifier::Wait(time_t timeout_ms) {
  int ret = lock_fd_epoll_.Lock();
  if (true!=ret) return -1;

  ret = epoll_wait(fd_epoll_, epoll_events_, kMaxEpollEvents, timeout_ms);
  if (ret>0) {
    for (int i=0; i<ret; ++i) {
      char buf[1];
      while (read(epoll_events_[i].data.fd, buf, sizeof(buf)) > 0) 
        ;
    }
  }
  lock_fd_epoll_.Unlock();
  return ret;
}

DefaultNotifier::~DefaultNotifier() {
  MAG_DELETE_ARRAY(epoll_events_)
  if (0!=fd_epoll_) close(fd_epoll_);
  if (0!=pthread_key_) pthread_key_delete(pthread_key_);
}

}
