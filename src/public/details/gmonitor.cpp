#include "../gmonitor.h"

#ifdef MONITOR_TYPE

namespace magneto {

MONITOR_TYPE* GMonitor::monitor_ = NULL; 
bool GMonitor::owner_;

bool GMonitor::Init(const std::string& conf_path, MONITOR_TYPE* monitor) {
  if (NULL!=monitor) {
    owner_=false;
    monitor_=monitor;
    return true;
  } else {
    owner_=true;
    monitor_ = new MONITOR_TYPE;
    return monitor_->Init(conf_path);
  }
}

void GMonitor::Tini() {
  if (owner_ && NULL!=monitor_) {
    delete monitor_;
  }
}

}

#endif
