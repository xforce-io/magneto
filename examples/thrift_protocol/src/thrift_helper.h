#pragma once

#include <tr1/memory>

#include "thrift/config.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/transport/TTransportUtils.h"

template <typename ThriftStruct>
void ThriftToBuf(const ThriftStruct& ts, std::string& out) {
  using namespace ::apache::thrift::transport;
  using namespace ::apache::thrift::protocol;

  boost::shared_ptr<TMemoryBuffer> buffer(new TMemoryBuffer);
  boost::shared_ptr<TTransport> trans(buffer);
  TBinaryProtocol protocol(trans);
  ts.write(&protocol);
  uint8_t* buf;
  uint32_t size;
  buffer->getBuffer(&buf, &size);
  out.assign(RCAST<char*>(buf), size_t(size));
}

template <typename ThriftStruct>
void BufToThrift(const char* buf, size_t size, ThriftStruct& ts) {
  using namespace ::apache::thrift::transport;
  using namespace ::apache::thrift::protocol;
 
  boost::shared_ptr<TMemoryBuffer> buffer(new TMemoryBuffer);
  buffer->write(RCAST<const uint8_t*>(buf), uint32_t(size));
  boost::shared_ptr<TTransport> trans(buffer);
  TBinaryProtocol protocol(trans);
  ts.read(&protocol);
}
