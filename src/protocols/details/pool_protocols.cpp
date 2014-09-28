#include "../pool_protocols.h"

namespace magneto {

ThreadPrivatePool<ProtocolWrite> PoolProtocols::pool_protocol_write_;
ThreadPrivatePool<ProtocolRead> PoolProtocols::pool_protocol_read_;

}
