#pragma once

#include "public.h"
#include "pool_conns_per_remote.h"
#include "../model.h"
#include "../confs/conf_services.h"

namespace xforce { namespace magneto {

class ConnsMgr {
 private:
  typedef std::vector<Remote> Remotes;
  typedef std::vector<size_t> Weights;
  typedef CloseHashmap<std::string, PoolConnsPerRemote*> Container;
  typedef std::tr1::unordered_set<std::string> SetRemotes;

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

  inline std::pair<int, bool> GetFd(const std::string& remote);

  inline void FreeFd(const std::string& remote, int fd);

  inline void ReportStatus(const std::string& remote, bool status);

  virtual ~ConnsMgr();

 private:
  inline std::pair<int, bool> GetFd_(const std::string& remote);
  inline const Remote* GetRemote_(
      const Service& service, 
      const Remotes& fail_remotes) const;

  inline const Remote* GetRemoteNormal_(
      const Service& service,
      const Remotes& fail_remotes) const;

  const Remote* GetRemoteWeight_(const Service& service, const Remotes& remotes) const;

  inline bool IsHealthy_(const Remote& remote) const;
  inline bool FailedBefore_(const Remote& remote, const Remotes& fail_remotes) const;

  inline const Remote* GetRemoteFromName_(const std::string& remote) const;

 private:
  time_t long_conn_keepalive_sec_;
  Container conns_; 
  ConfServices::Remotes remotes_;

  SetRemotes unhealthy_remotes_;

  mutable Remotes tmp_remotes_;
  mutable Weights tmp_weights_;
};

std::pair<int, bool> ConnsMgr::GetFd(
    const Service& service,
    const Remotes& fail_remotes,
    const Remote*& selected_remote) {
  selected_remote = GetRemote_(service, fail_remotes);
  return GetFd_(selected_remote->name);
}

std::pair<int, bool> ConnsMgr::GetFd(const std::string& remote) {
  return GetFd_(remote);
}

void ConnsMgr::FreeFd(const std::string& remote, int fd) {
  DEBUG("free fd " << fd);
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

void ConnsMgr::ReportStatus(const std::string& remote, bool status) {
  PoolConnsPerRemote** pool = conns_.Get(remote);
  if (likely(NULL!=pool)) {
    bool old_status = (*pool)->IsHealthy();
    (*pool)->ReportStatus(status);
    bool new_status = (*pool)->IsHealthy();
    if (old_status && !new_status) {
      unhealthy_remotes_.insert(remote);
    } else if (!old_status && new_status) {
      unhealthy_remotes_.erase(remote);
    }
  }
}

std::pair<int, bool> ConnsMgr::GetFd_(const std::string& remote_name) {
  PoolConnsPerRemote** pool = conns_.Get(remote_name);
  if (unlikely(NULL==pool)) {
    const Remote* remote = GetRemoteFromName_(remote_name);
    if (NULL==remote) {
      return std::make_pair(-1, false);
    }

    XFC_NEW_DECL(
        pool_conns_per_remote, 
        PoolConnsPerRemote, 
        PoolConnsPerRemote(*remote, long_conn_keepalive_sec_))
    conns_.Insert(remote_name, pool_conns_per_remote);
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
      XFC_BUG(true)
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
  SetRemotes::const_iterator iter;
  for (iter = unhealthy_remotes_.begin(); iter != unhealthy_remotes_.end(); ++iter) {
    if (*iter == remote.name) {
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
    if (remote.name == iter->name) {
      return true;
    }
  }
  return false;
}

const Remote* ConnsMgr::GetRemoteFromName_(const std::string& remote) const {
  ConfServices::Remotes::const_iterator iter = remotes_.find(remote);
  if (iter != remotes_.end()) {
    return &(iter->second);
  } else {
    return NULL;
  }
}

}}
