#pragma once

#include <tr1/memory>

#ifdef MAGNETO_THRIFT_SUPPORT

#include "thrift/config.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/transport/TTransportUtils.h"

using ::apache::thrift::protocol::TBinaryProtocol;
using ::apache::thrift::transport::TTransport;
using ::apache::thrift::transport::TMemoryBuffer;

namespace xforce { namespace magneto {

class TSimpleProtocol : public TBinaryProtocol {
 private:
  typedef TBinaryProtocol Father;
  
 public: 
  inline TSimpleProtocol(); 
  inline TSimpleProtocol(char* buf, size_t size); 
  inline TMemoryBuffer& GetMemBuf() { return *SCAST<TMemoryBuffer*>(trans_); }
};

TSimpleProtocol::TSimpleProtocol() :
  Father(boost::shared_ptr<TTransport>(new TMemoryBuffer)) {}

TSimpleProtocol::TSimpleProtocol(char* buf, size_t size) :
  Father(boost::shared_ptr<TTransport>(
      new TMemoryBuffer(RCAST<uint8_t*>(buf), size_t(size)))) {}

template <typename ThriftStruct>
void ThriftToBuf(const ThriftStruct& ts, std::string& out) {
  TSimpleProtocol protocol;
  ts.write(&protocol);

  uint8_t* buf;
  uint32_t size;
  protocol.GetMemBuf().getBuffer(&buf, &size);
  out.assign(RCAST<char*>(buf), size_t(size));
}

template <typename ThriftStruct>
void BufToThrift(const char* buf, size_t size, ThriftStruct& ts) {
  TSimpleProtocol protocol(CCAST<char*>(buf), size);
  ts.read(&protocol);
}

}}

#endif
