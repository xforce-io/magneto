#pragma once

#include "public.h"
#include "conf_normal.h"
#include "versioned_conf_services.h"

namespace xforce { namespace magneto {

class ConfServices;

class Confs {
 public:
  bool Init(const std::string& filedir);

  inline const ConfServices* GetConfServices(size_t version=-1) const;
  const ConfNormal& GetConfNormal() const { return conf_normal_; }

 private: 
  ConfNormal conf_normal_;
  VersionedConfServices versioned_conf_services_;
};

const ConfServices* Confs::GetConfServices(size_t version) const{
  return versioned_conf_services_.GetConf(version);
}

}}
