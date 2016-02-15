#include "../confs.h"

namespace xforce { namespace magneto {

bool Confs::Init(const std::string& filedir) {
  filedir_ = filedir;
  last_conf_reload_sec_ = Time::GetCurrentSec();
  return conf_normal_.Init(filedir) && versioned_conf_services_.Load(filedir, conf_normal_);
}

bool Confs::Reload() {
  time_t cur_sec = Time::GetCurrentSec();
  if (cur_sec - last_conf_reload_sec_ <= kMinReloadIntervalSec) {
    return true;
  }

  bool ret = versioned_conf_services_.Load(filedir_, conf_normal_);
  if (ret) {
    last_conf_reload_sec_ = cur_sec;
  }
  return ret;
}

}}
