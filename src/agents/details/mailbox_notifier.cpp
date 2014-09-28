#include "../mailbox_notifier.h"

namespace magneto {

MailboxNotifier::MailboxNotifier(Driver& driver) :
  driver_(&driver),
  event_ctx_(NULL) {
  MAG_STOP(0 != pthread_key_create(&pthread_key_, NULL))
}

int MailboxNotifier::Notify() {
  int* fd = RCAST<int*>(pthread_getspecific(pthread_key_));
  if (unlikely(NULL==fd)) {
    int pipefd[2];
    int ret = pipe(pipefd);
    if (0!=ret || !IOHelper::SetNonBlock(pipefd[0]) || !IOHelper::SetNonBlock(pipefd[1])) {
      return -1;
    }

    MAG_NEW_DECL(event_ctx_, EventCtx, EventCtx)
    event_ctx_->BuildForMailboxNotify(pipefd[0]);
    driver_->RegEvent(pipefd[0], Driver::kAddEvent, Driver::kIn, event_ctx_, -1);

    MAG_NEW_DECL(send_fd, int,  int(pipefd[1]))
    ret = pthread_setspecific(pthread_key_, send_fd);
    if (0!=ret) {
      driver_->RegEventDel(*send_fd);
      MAG_DELETE(send_fd)
      return -2;
    }
    fd=send_fd;
  }

  char buf[1];
  ssize_t ret = write(*fd, buf, 1);
  return 1==ret ? 0 : -3;
}

MailboxNotifier::~MailboxNotifier() {
  int* fd = RCAST<int*>(pthread_getspecific(pthread_key_));
  if (unlikely(NULL!=fd)) {
    MAG_DELETE(fd)
  }
  MAG_DELETE(event_ctx_)
}

}
