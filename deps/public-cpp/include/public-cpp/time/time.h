#pragma once

#include "../common.h"

namespace xforce {

class Time {
 public:
  inline static time_t GetCurrentSec(bool update);
  inline static time_t GetCurrentMsec(bool update);
  inline static time_t GetCurrentUsec(bool update);
  inline static void UpdateTimer();
  static void UninteruptbleSleep(time_t sec);

 private:
  static int64_t time_;
};

class Timer {
 public:
  inline Timer();
  inline void Start(bool update);
  inline void Stop(bool update);
  time_t TimeUs() const { return stop_-start_; }
  time_t TimeMs() const { return (stop_-start_)/1000; }
  time_t TimeSec() const { return (stop_-start_)/1000000; }

 private:
  time_t start_, stop_;
};

time_t Time::GetCurrentSec(bool update) { 
  if(update || 0==time_) UpdateTimer();
  return time_/1000000; 
}

time_t Time::GetCurrentMsec(bool update) {
  if(update || 0==time_) UpdateTimer();
  return time_/1000; 
}

time_t Time::GetCurrentUsec(bool update) { 
  if(update || 0==time_) UpdateTimer();
  return time_;
}

void Time::UpdateTimer() {
  timeval t;
  gettimeofday(&t, NULL);
  time_ = t.tv_sec*1000000 + t.tv_usec;
}

Timer::Timer() {
  start_ = Time::GetCurrentUsec(true);
}

void Timer::Start(bool update) { 
  start_ = Time::GetCurrentUsec(update);
}

void Timer::Stop(bool update) { 
  stop_ = Time::GetCurrentUsec(update);
}

}
