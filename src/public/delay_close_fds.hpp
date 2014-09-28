#pragma once

#include "common.h"
#include "io_helper.h"
#include "time/time.h"

namespace magneto {

class DelayCloseFds {
 private:
  typedef std::vector< std::pair<int, time_t> > Container;
  
 public:
  inline DelayCloseFds();

  void Close(int fd, time_t delay_time_sec=0);

  inline virtual ~DelayCloseFds();
 
 private: 
  Container container_;
  time_t last_check_sec_;
};

DelayCloseFds::DelayCloseFds() :
  last_check_sec_(0) {}

inline void DelayCloseFds::Close(int fd, time_t delay_time_sec) {
  if (0==delay_time_sec) {
    IOHelper::Close(fd);
  } else {
    container_.push_back(std::make_pair(fd, Time::GetCurrentSec(false) + delay_time_sec));
  }

  if (unlikely(Time::GetCurrentSec(false) != last_check_sec_)) {
    Container::iterator iter;
    for (iter = container_.begin(); iter != container_.end();) {
      if (iter->second < Time::GetCurrentSec(false)) {
        IOHelper::Close(iter->first);
        container_.erase(iter);
      } else {
        ++iter;
      }
    }
    last_check_sec_ = Time::GetCurrentSec(false);
  }
}

DelayCloseFds::~DelayCloseFds() {
  Container::iterator iter;
  for (iter = container_.begin(); iter != container_.end(); ++iter) {
    IOHelper::Close(iter->first);
  }
}

}
