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

int ProtocolReadRapid::Read(int fd) {
  bool has_bytes_read=false;
  for (;;) {
    size_t bytes_left;
    if (read_header_) {
      bytes_left = sizeof(RapidHeader) - buffer_.Len();
      int ret = IOHelper::ReadNonBlock(fd, buffer_.Stop(), bytes_left);
      if (ret>0) {
        has_bytes_read=true;
        buffer_.SetLen(buffer_.Len() + ret);
        if ( sizeof(RapidHeader) == buffer_.Len() ) {
          read_header_=false;

          RapidHeader* header = CCAST<RapidHeader*>(Header());
          header->size = ntohl(header->size);
          buffer_.Reserve(sizeof(RapidHeader) + header->size);
          continue;
        }
      } else if (0==ret) {
        return has_bytes_read ? bytes_left : kEnd;
      } else {
        return ret;
      }
    } else {
      bytes_left = sizeof(RapidHeader) + CCAST<RapidHeader*>(Header())->size - buffer_.Len();
      int ret = IOHelper::ReadNonBlock(fd, buffer_.Stop(), bytes_left);
      if (ret>0) {
        has_bytes_read=true;
        buffer_.SetLen(buffer_.Len() + ret);
        return bytes_left-ret;
      } else if (0==ret) {
        return has_bytes_read ? bytes_left : kEnd;
      } else {
        return ret;
      }
    }
  }
}

}}
