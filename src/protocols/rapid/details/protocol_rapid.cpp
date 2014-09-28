#include "../protocol_rapid.h"
#include "../../public.h"

namespace magneto {

int ProtocolWriteRapid::Write(int fd) {
  if (write_header_) {
    int ret = IOHelper::WriteVec2NonBlock(fd, tmp_iov_);
    if (ret>0) {
      size_t bytes_written = ret + sizeof(header_) - tmp_iov_[0].iov_len;
      if ( bytes_written == sizeof(header_) + size_ ) {
        return 0;
      } else if ( bytes_written >= sizeof(header_) ) {
        write_header_=false;
        tmp_iov_[1].iov_base = RCAST<char*>(tmp_iov_[1].iov_base) + bytes_written - sizeof(header_);
        tmp_iov_[1].iov_len = sizeof(header_) + size_ - bytes_written;
      } else {
        tmp_iov_[0].iov_base = RCAST<char*>(tmp_iov_[0].iov_base) + bytes_written;
        tmp_iov_[0].iov_len = tmp_iov_[0].iov_len - bytes_written;
      }
      return sizeof(header_) + size_ - bytes_written;
    } else {
      return 0==ret ? kEnd : ret;
    }
  } else {
    int ret = IOHelper::WriteNonBlock(fd, tmp_iov_[1].iov_base, tmp_iov_[1].iov_len);
    if (ret>0) {
      tmp_iov_[1].iov_len -= ret;
      return tmp_iov_[1].iov_len;
    } else {
      return 0==ret ? kEnd : ret;
    }  
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



}
