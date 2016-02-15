#pragma once

#include "public.h"
#include "conf_normal.h"
#include "versioned_conf_services.h"

namespace xforce { namespace magneto {

class ConfServices;
class Remote;

class Confs {
 public:
  static const time_t kMinReloadIntervalSec = 1;
  
 public:
  bool Init(const std::string& filedir);

  bool Reload();

  inline const ConfServices* GetConfServices(size_t version=-1) const;
  const ConfNormal& GetConfNormal() const { return conf_normal_; }

 private: 
  std::string filedir_;
  ConfNormal conf_normal_;
  VersionedConfServices versioned_conf_services_;
  time_t last_conf_reload_sec_;
};

const ConfServices* Confs::GetConfServices(size_t version) const{
  return versioned_conf_services_.GetConf(version);
}

}}
