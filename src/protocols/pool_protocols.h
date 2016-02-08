#pragma once 

#include "public.h"
#include "protocols.h"

namespace xforce { namespace magneto {

class PoolProtocols {
 public:
  inline static Protocol::Category GetCategory(const std::string& category);

  inline static ProtocolWrite* GetWrite(Protocol::Category category);
  inline static ProtocolRead* GetRead(Protocol::Category category);

  template <typename Protocol>
  inline static ProtocolWrite* GetWrite();

  template <typename Protocol>
  inline static ProtocolRead* GetRead();

  inline static void FreeWrite(ProtocolWrite* protocol);
  inline static void FreeRead(ProtocolRead* protocol);
 
 private:
  static ThreadPrivatePool<ProtocolWrite> pool_protocol_write_;
  static ThreadPrivatePool<ProtocolRead> pool_protocol_read_;
};

Protocol::Category PoolProtocols::GetCategory(const std::string& category) {
  if ("rapid" == category) {
    return Protocol::kRapid;
  } else if ("ping" == category) {
    return Protocol::kPing;
  } else if ("redis" == category) {
    return Protocol::kRedis;
#ifdef MAGNETO_THRIFT_SUPPORT
  } else if ("thrift" == category) {
    return Protocol::kThrift;
#endif
#ifdef MAGNETO_PROTOBUF_SUPPORT
  } else if ("protobuf" == category) {
    return Protocol::kProtobuf;
#endif
  } else {
    return Protocol::kInvalid;
  }
}

ProtocolWrite* PoolProtocols::GetWrite(Protocol::Category category) {
  switch (category) {
    case Protocol::kPing :
      return GetWrite<ProtocolWritePing>();
    case Protocol::kRapid :
      return GetWrite<ProtocolWriteRapid>();
    case Protocol::kRedis :
      return GetWrite<ProtocolWriteRedis>();
#ifdef MAGNETO_THRIFT_SUPPORT
    case Protocol::kThrift :
      return GetWrite<ProtocolWriteThrift>();
#endif
#ifdef MAGNETO_PROTOBUF_SUPPORT
    case Protocol::kProtobuf:
      return GetWrite<ProtocolWriteProtobuf>();
#endif
    default :
      XFC_BUG(true)
      return NULL;
  }
}

ProtocolRead* PoolProtocols::GetRead(Protocol::Category category) {
  switch (category) {
    case Protocol::kPing :
      return GetRead<ProtocolReadPing>();
    case Protocol::kRapid :
      return GetRead<ProtocolReadRapid>();
    case Protocol::kRedis :
      return GetRead<ProtocolReadRedis>();
#ifdef MAGNETO_THRIFT_SUPPORT
    case Protocol::kThrift :
      return GetRead<ProtocolReadThrift>();
#endif
#ifdef MAGNETO_PROTOBUF_SUPPORT
    case Protocol::kProtobuf:
      return GetRead<ProtocolReadProtobuf>();
#endif
    default :
      XFC_BUG(true)
      return NULL;
  }
}

template <typename Protocol>
ProtocolWrite* PoolProtocols::GetWrite() {
  return pool_protocol_write_.Get<Protocol>();
}

template <typename Protocol> 
ProtocolRead* PoolProtocols::GetRead() {
  return pool_protocol_read_.Get<Protocol>();
}

void PoolProtocols::FreeWrite(ProtocolWrite* protocol) {
  pool_protocol_write_.Free(*protocol);
}

void PoolProtocols::FreeRead(ProtocolRead* protocol) {
  pool_protocol_read_.Free(*protocol);
}

}}
