#pragma once

#ifdef MONITOR_TYPE

#include "monitor/monitor.h"

namespace xforce {

class GMonitor {
 public:
  static bool Init(const std::string& conf_path, MONITOR_TYPE* monitor=NULL);
  static MONITOR_TYPE& Get() { return *monitor_; }
  static void Tini();

 private:
  static MONITOR_TYPE* monitor_; 
  static bool owner_;
};

}

#endif
