#include "../public.h"
#include "../../protocols/pool_protocols.h"

namespace xforce { namespace magneto {

bool ConfHelper::ParseServiceTag(
    const std::string& service_tag,
    std::string& service_name,
    Protocol::Category& category) {
  size_t pos_sep = service_tag.find("|"); 
  if (std::string::npos == pos_sep || 0 == pos_sep) {
    return false;
  }

  service_name = service_tag.substr(0, pos_sep); 
  category = PoolProtocols::GetCategory(service_tag.substr(pos_sep+1));
  if (Protocol::kInvalid == category) {
    return false;
  }
  return true;
}

}}
