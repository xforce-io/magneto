#include "../protocol.h"

namespace xforce { namespace magneto {

void ProtocolRead::Reset(const ListenAddr* listen_addr) {
  if (NULL!=listen_addr) {
    req_info_.listen_addr = listen_addr;
  }
}

}}
