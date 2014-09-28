#include "../time.h"

namespace magneto {

int64_t Time::time_ = 0;

void Time::UninteruptbleSleep(time_t sec) {
  timespec req, rem;
  req.tv_sec = sec;
  req.tv_nsec = 0;
  int ret = nanosleep(&req, &rem);
  while (0!=ret) {
    req=rem;
    ret = nanosleep(&req, &rem);
  }
}

}
