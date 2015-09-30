#pragma once

#include "common.h"
#include "heap.hpp"
#include "time/time.h"

namespace xforce {

template <typename Context, typename LockPolicy=NoLock>
class EventsDriver {
 public: 
  enum Op {
    kAddEvent,
    kModEvent,
    kDelEvent,
  };

  enum Event {
    kIn=0,
    kOut=1,
    kErr=2,
  };

  struct EventCtx {
    Context* context;
    size_t sign_timeo;
  };

  struct EventTimeo {
    time_t expire_time_ms;
    int fd;
  };

 public:
  static const size_t kMaxNumEvents=100000;

 public: 
  EventsDriver(bool level_trigger=true);

  inline int Wait(time_t timeo_ms);
  bool RegEvent(int fd, Op op, Event event, Context* context, int timeo_ms);
  bool RegEventDel(int fd);
  inline void CheckReadyEvent(size_t index_events_ready, Event& event, Context*& context);
  inline const std::vector<Context*>* RemoveTimeouts();
  std::vector<Context*>* ClearAll();

  virtual ~EventsDriver();

 private: 
  bool Init_();

 private:
  uint32_t event_in_;
  uint32_t event_out_;

  int fd_epoll_;
  epoll_event* events_ready_;

  LockPolicy lock_;

  template <typename T>
  struct GetExpireTime {
    time_t operator()(const T& t) const { return t.expire_time_ms; }
  };

  SimpleHeap<
    EventTimeo,
    HeapCategory::kMinHeap,
    GetExpireTime<EventTimeo> > heap_timeo_;
  std::vector<EventCtx> fd_to_event_ctx_;  
  std::vector<Context*> tmp_timeo_contexts_;
  std::vector<Context*> tmp_all_contexts_;

  bool init_;
};

template <typename Context, typename LockPolicy>
EventsDriver<Context, LockPolicy>::EventsDriver(bool level_trigger) : 
  event_in_(EPOLLIN|EPOLLHUP|EPOLLERR),
  event_out_(EPOLLOUT|EPOLLHUP|EPOLLERR),
  events_ready_(NULL), 
  init_(false) {
  if (!level_trigger) {
    event_in_ |= EPOLLET;
    event_out_ |= EPOLLET;
  }
}

template <typename Context, typename LockPolicy>
int EventsDriver<Context, LockPolicy>::Wait(time_t timeo_ms) {
  XFC_RAII_INIT(-1)

  int64_t time_to_wait_ms=timeo_ms;
  if (0 != heap_timeo_.Size()) {
    time_to_wait_ms = std::min(
        heap_timeo_.Top()->expire_time_ms - Time::GetCurrentMsec(false),
        time_to_wait_ms);
    if (time_to_wait_ms<0) time_to_wait_ms=0;
  }

  int ret = epoll_wait(fd_epoll_, events_ready_, kMaxNumEvents, time_to_wait_ms);
  Time::UpdateTimer();
  return ret;
}

template <typename Context, typename LockPolicy>
bool EventsDriver<Context, LockPolicy>::RegEvent(int fd, Op op, Event direction, Context* context, int timeo_ms) {
  XFC_RAII_INIT(-1)

  XFC_BUG(NULL==context)

  int ret=-1;
  EventTimeo event_timeo;

  if (fd >= SCAST<int>(fd_to_event_ctx_.size())) {
    size_t old_size = fd_to_event_ctx_.size();
    fd_to_event_ctx_.resize(fd+1);
    for (size_t i=old_size; i < fd_to_event_ctx_.size(); ++i) {
      fd_to_event_ctx_[i].context = NULL;
    }
  }

  epoll_event ev;
  ev.events = (kIn==direction) ? event_in_ : event_out_;
  ev.data.fd = fd;

  switch (op) {
    case kAddEvent :
      fd_to_event_ctx_[fd].context = context;

      if (timeo_ms>=0) { 
        event_timeo = (struct EventTimeo){Time::GetCurrentMsec(false) + timeo_ms, fd};
        heap_timeo_.Insert(event_timeo, fd_to_event_ctx_[fd].sign_timeo);
      } else {
        fd_to_event_ctx_[fd].sign_timeo = 0;
      }

      XFC_FAIL_HANDLE(!lock_.Lock())
      ret = epoll_ctl(fd_epoll_, EPOLL_CTL_ADD, fd, &ev);
      lock_.Unlock();

      XFC_FAIL_HANDLE_FATAL(0!=ret, "fail_epoll_ctl_add fd[" << fd << "] reason[" << errno << "]")
      break;
    case kModEvent :
      if (timeo_ms>=0) { 
        event_timeo = (struct EventTimeo){Time::GetCurrentMsec(false) + timeo_ms, fd};
        if (0 != fd_to_event_ctx_[fd].sign_timeo) {
          heap_timeo_.Update(fd_to_event_ctx_[fd].sign_timeo, event_timeo);
        } else {
          heap_timeo_.Insert(event_timeo, fd_to_event_ctx_[fd].sign_timeo);
        }
      }

      XFC_FAIL_HANDLE(!lock_.Lock())
      ret = epoll_ctl(fd_epoll_, EPOLL_CTL_MOD, fd, &ev);
      lock_.Unlock();

      XFC_FAIL_HANDLE_FATAL(0!=ret, "fail_epoll_ctl_mod fd[" << fd << "] reason[" << strerror(errno) << "]")
      break;
    default : // kDelEvent
      ret = RegEventDel(fd);
      XFC_FAIL_HANDLE(0!=ret)

      fd_to_event_ctx_[fd].context = NULL;
      break;
  }
  return true;

  ERROR_HANDLE:
  RegEventDel(fd);
  return false;
}

template <typename Context, typename LockPolicy>
bool EventsDriver<Context, LockPolicy>::RegEventDel(int fd) {
  if (0 != fd_to_event_ctx_[fd].sign_timeo) {
    heap_timeo_.Erase(fd_to_event_ctx_[fd].sign_timeo);
    fd_to_event_ctx_[fd].sign_timeo = 0;
  }

  if (!lock_.Lock()) return false;
  int ret = epoll_ctl(fd_epoll_, EPOLL_CTL_DEL, fd, NULL);
  lock_.Unlock();

  if (0!=ret) {
    FATAL("fail_epoll_ctl_del fd[" << fd << "] reason[" << strerror(errno) << "]");
    return false;
  }

  fd_to_event_ctx_[fd].context = NULL;
  return true;
}

template <typename Context, typename LockPolicy>
void EventsDriver<Context, LockPolicy>::CheckReadyEvent(
    size_t index_events_ready, 
    Event& event, 
    Context*& context) {
  XFC_RAII_INIT()

  event = (EPOLLOUT == events_ready_[index_events_ready].events ? kOut : 
      (EPOLLIN == events_ready_[index_events_ready].events ? kIn : kErr));
  context = fd_to_event_ctx_[events_ready_[index_events_ready].data.fd].context;
  XFC_BUG(NULL==context)
}

template <typename Context, typename LockPolicy>
const std::vector<Context*>* EventsDriver<Context, LockPolicy>::RemoveTimeouts() { 
  XFC_RAII_INIT(NULL)

  tmp_timeo_contexts_.clear();
  EventTimeo* event_timeo = heap_timeo_.Top();
  while ( NULL!=event_timeo 
      && event_timeo->expire_time_ms < Time::GetCurrentMsec(false) ) {
    if (NULL != fd_to_event_ctx_[event_timeo->fd].context) {
      tmp_timeo_contexts_.push_back(fd_to_event_ctx_[event_timeo->fd].context);
    }

    bool ret = RegEventDel(event_timeo->fd);
    if (!ret) { FATAL("fail_reg_event"); }

    event_timeo = heap_timeo_.Top();
  }
  return &tmp_timeo_contexts_;
}

template <typename Context, typename LockPolicy>
std::vector<Context*>* EventsDriver<Context, LockPolicy>::ClearAll() {
  XFC_RAII_INIT(NULL)

  tmp_all_contexts_.clear();
  for (size_t i=0; i < fd_to_event_ctx_.size(); ++i) {
    if (NULL != fd_to_event_ctx_[i].context) {
      tmp_all_contexts_.push_back(fd_to_event_ctx_[i].context);
      bool ret = RegEventDel(i);
      if (!ret) { FATAL("fail_reg_event"); }
    }
  }
  return &tmp_all_contexts_;
}

template <typename Context, typename LockPolicy>
EventsDriver<Context, LockPolicy>::~EventsDriver() { 
  ClearAll();
  XFC_DELETE_ARRAY(events_ready_) 
}

template <typename Context, typename LockPolicy>
bool EventsDriver<Context, LockPolicy>::Init_() {
  fd_epoll_ = epoll_create(1);
  XFC_FAIL_HANDLE_FATAL(fd_epoll_<=0, "error_create_fd error[" << strerror(errno) << "]")

  XFC_NEW(events_ready_, epoll_event [kMaxNumEvents])
  init_=true;
  return true;

  ERROR_HANDLE:
  return false;
}

}
