#include "../versioned_conf_services.h"

namespace xforce { namespace magneto {

bool VersionedConfServices::Load(const std::string& filedir, const ConfNormal& conf_normal) {
  XFC_NEW_DECL(conf, ConfServices, ConfServices)
  bool ret = conf->Init(filedir, conf_normal);
  if (!ret) {
    WARN("fail_load[" << filedir << "]");
    return false;
  }

  confs_.Change(*conf);
  return true;
}

}}
