#pragma once

#include "public.h"
#include "pool_conns_per_remote.h"
#include "../model.h"
#include "../confs/conf_services.h"

namespace magneto {

class ConnsMgr {
 private:
  typedef std::vector<Remote> Remotes;
  typedef std::vector<size_t> Weights;
  typedef CloseHashmap<Remote, PoolConnsPerRemote*, Remote::EqRemote, Remote::HashRemote> Container;
  typedef std::tr1::unordered_set<Remote, Remote::HashRemote, Remote::EqRemote> SetGuys;

 public:
  ConnsMgr(time_t long_conn_keepalive_sec);

  void ConfigRemotes(const ConfServices::Remotes& remotes);

  /*
   * @return: <fd, whether need event>
   */
  inline std::pair<int, bool> GetFd(
      IN const Service& service,
      IN const Remotes& fail_remotes,
      OUT const Remote*& selected_remote);

  inline void FreeFd(const Remote& remote, int fd);

  inline void ReportStatus(const Remote& remote, bool status);

  virtual ~ConnsMgr();

 private:
  inline std::pair<int, bool> GetFd_(const Remote& remote);
  inline const Remote* GetRemote_(
      const Service& service, 
      const Remotes& fail_remotes) const;

  inline const Remote* GetRemoteNormal_(
      const Service& service,
      const Remotes& fail_remotes) const;

  const Remote* GetRemoteWeight_(const Service& service, const Remotes& remotes) const;

  inline bool IsHealthy_(const Remote& remote) const;
  inline bool FailedBefore_(const Remote& remote, const Remotes& fail_remotes) const;

 private:
  time_t long_conn_keepalive_sec_;
  Container conns_; 
  SetGuys unhealthy_guys_;

  mutable Remotes tmp_remotes_;
  mutable Weights tmp_weights_;
};

std::pair<int, bool> ConnsMgr::GetFd(
    const Service& service,
    const Remotes& fail_remotes,
    const Remote*& selected_remote) {
  selected_remote = GetRemote_(service, fail_remotes);
  return GetFd_(*selected_remote);
}

void ConnsMgr::FreeFd(const Remote& remote, int fd) {
  PoolConnsPerRemote** pool = conns_.Get(remote);
  if (unlikely(NULL==pool)) {
    IOHelper::Close(fd);
    return;
  }

  PoolConnsPerRemote& pool_conns_per_remote = **pool;
  if (pool_conns_per_remote.GetNumLongConnsPerRemote() > 0) {
    pool_conns_per_remote.Put(fd);
  } else {
    IOHelper::Close(fd);
  }
}

void ConnsMgr::ReportStatus(const Remote& remote, bool status) {
  PoolConnsPerRemote** pool = conns_.Get(remote);
  if (likely(NULL!=pool)) {
    bool old_status = (*pool)->IsHealthy();
    (*pool)->ReportStatus(status);
    bool new_status = (*pool)->IsHealthy();
    if (old_status && !new_status) {
      unhealthy_guys_.insert(remote);
    } else if (!old_status && new_status) {
      unhealthy_guys_.erase(remote);
    }
  }
}

std::pair<int, bool> ConnsMgr::GetFd_(const Remote& remote) {
  PoolConnsPerRemote** pool = conns_.Get(remote);
  if (unlikely(NULL==pool)) {
    MAG_NEW_DECL(pool_conns_per_remote, PoolConnsPerRemote, PoolConnsPerRemote(remote, long_conn_keepalive_sec_))
    conns_.Insert(remote, pool_conns_per_remote);
    return pool_conns_per_remote->Get();
  }
  return (*pool)->Get();
}

const Remote* ConnsMgr::GetRemote_(
    const Service& service, 
    const Remotes& fail_remotes) const {
  switch (service.service_strategy) {
    case ServiceStrategy::kNormal :
      return GetRemoteNormal_(service, fail_remotes);
    case ServiceStrategy::kWeight :
      return GetRemoteWeight_(service, fail_remotes);
    default :
      MAG_BUG(true)
  }
  return NULL;
}

const Remote* ConnsMgr::GetRemoteNormal_(
    const Service& service,
    const Remotes& fail_remotes) const {
  Service::Remotes::const_iterator iter;
  for (iter = service.remotes.begin(); iter != service.remotes.end(); ++iter) {
    if (IsHealthy_(*iter) && !FailedBefore_(*iter, fail_remotes)) {
      return &*iter;
    }
  }

  static unsigned int seed=0;
  return &(service.remotes[rand_r(&seed) % service.remotes.size()]);
}

bool ConnsMgr::IsHealthy_(const Remote& remote) const {
  SetGuys::const_iterator iter;
  for (iter = unhealthy_guys_.begin(); iter != unhealthy_guys_.end(); ++iter) {
    if (Remote::EqRemote()(*iter, remote)) {
      return false;
    }
  }
  return true;
}

bool ConnsMgr::FailedBefore_(
    const Remote& remote, 
    const Remotes& fail_remotes) const {
  Remotes::const_iterator iter;
  for (iter = fail_remotes.begin(); iter != fail_remotes.end(); ++iter) {
    if (Remote::EqRemote()(remote, *iter)) {
      return true;
    }
  }
  return false;
}

}
