#pragma once

#include "../model.h"

namespace magneto {

class PoolConnsPerRemote {
 private:
  static const size_t kDefaultMaxFd=10240;
  static const size_t kFullMarkHealth=100; 
 
 public:
  PoolConnsPerRemote(const Remote& remote, time_t long_conn_keepalive_sec);

  void SetNumLongConnsPerRemote(size_t long_conns);
  size_t GetNumLongConnsPerRemote() const { return remote_.long_conns; }

  inline void ReportStatus(bool status);
  size_t GetHealthMark() const { return health_mark_; }
  bool IsHealthy() const { return kFullMarkHealth==health_mark_; }

  /*
   * @return: <fd, needless to connect>
   */
  std::pair<int, bool> Get();
  void Put(int fd);

  virtual ~PoolConnsPerRemote() {}

 private:
  Remote remote_;
  time_t long_conn_keepalive_sec_;

  size_t health_mark_;
  std::vector<int> fds_;
  std::vector<time_t> expiretime_sec_;

  SpinLock lock_fds_;
};

void PoolConnsPerRemote::ReportStatus(bool status) {
  health_mark_ = status 
    ? std::min(health_mark_<<1, size_t(kFullMarkHealth))
    : std::max(health_mark_>>1, size_t(1));
}

}
