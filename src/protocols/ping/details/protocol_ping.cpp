#include "../protocol_ping.h"
#include "../../public.h"

namespace magneto {

int ProtocolWritePing::Write(int fd) {
  int ret = IOHelper::WriteNonBlock(fd, "p", 1);
  if (ret>0) {
    return 0;
  } else {
    return 0==ret ? kEnd : ret;
  }
}

int ProtocolReadPing::Read(int fd) {
  int ret = IOHelper::ReadNonBlock(fd, &buffer_, 1);
  if (ret>0) {
    return 0;
  } else {
    return 0==ret ? kEnd : ret;
  }
}

}
