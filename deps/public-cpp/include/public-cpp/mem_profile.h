#pragma once

#ifdef MEMPROFILE

namespace xforce {

class MemProfile {
 public:
  static void SetFlag(bool flag) { flag_=flag; }
  static bool GetFlag() { return flag_; }

 private:
  static bool flag_; 
};

}

#endif
