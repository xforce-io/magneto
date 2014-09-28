#pragma once

#include "../public.h"
#include "../protocols/protocol.h"

namespace magneto {

class ConfHelper {
 public: 
  static bool ParseServiceTag(
      const std::string& service_tag,
      std::string& service_name,
      Protocol::Category& category);
};

}
