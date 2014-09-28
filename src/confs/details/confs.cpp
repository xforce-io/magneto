#include "../confs.h"

namespace magneto {

bool Confs::Init(const std::string& filedir) {
  return conf_normal_.Init(filedir) && versioned_conf_services_.Load(filedir, conf_normal_);
}

}
