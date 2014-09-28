#pragma once

#include "public.h"
#include "conf_services.h"
#include "conf_normal.h"

namespace magneto {

class ConfServices;

class VersionedConfServices {
 public:
  bool Load(const std::string& filedir, const ConfNormal& conf_normal);
  size_t GetLatestConfVersion() const { return confs_.GetLatestVersion(); }
  inline const ConfServices* GetConf(size_t version=-1) const;
  inline ConfServices* GetConf(size_t version=-1);

 private:
  DelayDelPtr<ConfServices> confs_;
};

const ConfServices* VersionedConfServices::GetConf(size_t version) const {
  return confs_.Get(version);
}

ConfServices* VersionedConfServices::GetConf(size_t version) {
  return confs_.Get(version);
}

}
