#include "../protocol_rapid.h"
#include "../../public.h"

namespace xforce { namespace magneto {

int ProtocolWriteRapid::Write(int fd) {
  int ret = IOHelper::WriteVecNonBlock(fd, tmp_iovs_, tmp_num_iovs_);
  if (ret>0) {
    return 0==tmp_num_iovs_ ? 
      0 : 
      ( 1==tmp_num_iovs_ ? 
          tmp_iovs_[0].iov_len : 
          tmp_iovs_[0].iov_len + tmp_iovs_[1].iov_len );
  } else {
    return 0==ret ? kEnd : ret;
  }
}

}}
