#include "../pool_conns_per_remote.h"

#include "../public.h"

namespace xforce { namespace magneto {

PoolConnsPerRemote::PoolConnsPerRemote(const Remote& remote, time_t long_conn_keepalive_sec) :
  remote_(remote),
  long_conn_keepalive_sec_(long_conn_keepalive_sec),
  health_mark_(kFullMarkHealth) {
  expiretime_sec_.resize(kDefaultMaxFd);
  for (size_t i=0; i < expiretime_sec_.size(); ++i) {
    expiretime_sec_[i] = 0;
  }
}

void PoolConnsPerRemote::SetNumLongConnsPerRemote(size_t long_conns) {
  remote_.long_conns = long_conns;
}

std::pair<int, bool> PoolConnsPerRemote::Get() {
  if (likely(!fds_.empty())) {
    int fd=0;
    while (!lock_fds_.Lock())
      ;

    if (!fds_.empty()) {
      fd = fds_.back();
      fds_.pop_back();
    } 
    lock_fds_.Unlock();

    if (0!=fd) {
      if (IOHelper::CheckConn(fd)) {
        return std::make_pair(fd, true);
      } else {
        IOHelper::Close(fd);
      }
    }
  }
  std::pair<int, bool> result = IOHelper::Connect(remote_.addr.addr);
  return result;
}

void PoolConnsPerRemote::Put(int fd) {
  while (!lock_fds_.Lock())
    ;

  if (unlikely(fds_.size() >= remote_.long_conns)) {
    IOHelper::Close(fd);
    lock_fds_.Unlock();
    return;
  }

  /* resize expiretime_sec_ if nessesary */
  if (unlikely(fd >= SCAST<int64_t>(expiretime_sec_.size()))) {
    size_t old_size = expiretime_sec_.size();
    expiretime_sec_.resize(fd+1);
    for (size_t i=old_size; i < expiretime_sec_.size(); ++i) {
      expiretime_sec_[i] = 0;
    }
  }

  if (0 != expiretime_sec_[fd]) {
    if ( expiretime_sec_[fd] > Time::GetCurrentSec(false) ) {
      fds_.push_back(fd);
    } else {
      IOHelper::Close(fd);
      expiretime_sec_[fd] = 0;
    }
  } else {
    expiretime_sec_[fd] = Time::GetCurrentSec(false) + long_conn_keepalive_sec_;
    fds_.push_back(fd);
  }
  lock_fds_.Unlock();
}

}}
