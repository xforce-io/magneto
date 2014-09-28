#pragma once

#include "public.h"
#include "../event_ctx.h"

namespace magneto {

class MailboxNotifier {
 private:
  typedef EventsDriver<EventCtx, SpinLock> Driver;
  
 public:
  MailboxNotifier(Driver& driver);

  int Notify();
  int Wait(time_t /*time_ms*/) { return 1; }
  inline static void ClearReadBuf(int fd);

  virtual ~MailboxNotifier();

 private: 
  Driver* driver_;

  pthread_key_t pthread_key_;
  EventCtx* event_ctx_;
};

void MailboxNotifier::ClearReadBuf(int fd) {
  char buf[1];
  while (read(fd, buf, sizeof(buf)) > 0) ;
}

}
