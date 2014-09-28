#include "../conns_mgr.h"

namespace magneto {

ConnsMgr::ConnsMgr(time_t long_conn_keepalive_sec) :
  long_conn_keepalive_sec_(long_conn_keepalive_sec) {
  conns_.Init(LocalLimits::kNumRemotes*100);
}

void ConnsMgr::ConfigRemotes(const ConfServices::Remotes& remotes) {
  ConfServices::Remotes::const_iterator iter;
  for (iter = remotes.begin(); iter != remotes.end(); ++iter) {
    PoolConnsPerRemote** pool = conns_.Get(*iter);
    if (unlikely(NULL==pool)) {
      MAG_NEW_DECL(
          pool_conns_per_remote, 
          PoolConnsPerRemote,
          PoolConnsPerRemote(*iter, long_conn_keepalive_sec_))
      conns_.Insert(*iter, pool_conns_per_remote);
      pool = conns_.Get(*iter);
    }
    (*pool)->SetNumLongConnsPerRemote(iter->long_conns);
  }

  Container::Iterator iter_2;
  for (iter_2 = conns_.Begin(); !iter_2.IsEnd(); iter_2.Next()) {
    ConfServices::Remotes::const_iterator iter_remote = remotes.find(iter_2->first);
    if (remotes.end() == iter_remote) {
      PoolConnsPerRemote** pool = conns_.Get(iter_2->first);
      MAG_DELETE(*pool)

      unhealthy_guys_.erase(iter_2->first);
      conns_.Erase(iter_2->first);
    }  
  }
}

const Remote* ConnsMgr::GetRemoteWeight_(
    const Service& service,
    const Remotes& fail_remotes) const {
  static unsigned int seed=0;

  int weight=0;
  if (0 == unhealthy_guys_.size()) {
    int random = rand_r(&seed) % service.weight_all;
    for (size_t i=0; i < service.remotes.size(); ++i) {
      weight += service.remotes[i].weight;
      if (weight>random) {
        return &(service.remotes[i]);
      }
    }
  } else {
    tmp_remotes_.clear();
    tmp_weights_.clear();
    Service::Remotes::const_iterator iter;
    for (iter = service.remotes.begin(); iter != service.remotes.end(); ++iter) {
      if (!FailedBefore_(*iter, fail_remotes)) {
        tmp_remotes_.push_back(*iter);
      }
    }

    size_t weight_all=0;
    for (iter = tmp_remotes_.begin(); iter != tmp_remotes_.end(); ++iter) {
      size_t tmp_weight = iter->weight * (*conns_.Get(*iter))->GetHealthMark();
      weight_all+=tmp_weight;
      tmp_weights_.push_back(tmp_weight);
    }

    if (0 != tmp_weights_.size()) {
      int random = rand_r(&seed) % weight_all;
      for (size_t i=0; i < tmp_remotes_.size(); ++i) {
        weight += tmp_weights_[i];
        if (weight>random) {
          return &(tmp_remotes_[i]);
        }
      }
    } else {
      return &((service.remotes)[rand_r(&seed) % service.remotes.size()]);
    }
  }
  return NULL;
}

ConnsMgr::~ConnsMgr() {
  Container::Iterator iter;
  for (iter = conns_.Begin(); !iter.IsEnd(); iter.Next()) {
    MAG_DELETE(iter->second)
  }
}

}
