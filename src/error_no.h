#pragma once

#include "public-cpp/common.h"

namespace xforce { namespace magneto {

struct ErrorNo {
  static const int64_t kOk=0;
  static const int64_t kPartial=-1;
  static const int64_t kTimeout=-2;
  static const int64_t kIO=-3;
  static const int64_t kBroken=-4;
  static const int64_t kEncode=-5;
  static const int64_t kDecode=-6;
  static const int64_t kSwapCtx=-7;
  static const int64_t kQueueBusy=-8;
  static const int64_t kNotFound=-9;
  static const int64_t kOther=-100;
};

}}
