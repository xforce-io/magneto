#pragma once

#include "../public/common.h"
#include "../public_model.h"

namespace xforce { namespace magneto {

//typedef bool (*FuncReadChecker)(const char* buf, size_t count);

struct Addr {
 public: 
  sockaddr_in addr;

 public:
  inline bool Assign(const std::string& addr);
};

struct Protocol {
  enum Category {
    kRapid,
    kPing,
    kRedis,
#ifdef MAGNETO_THRIFT_SUPPORT
    kThrift,
#endif
    kHttp,
    kInvalid,
  };
};

struct ListenAddr {
  std::string name;
  Protocol::Category category;
  Addr addr;
};

struct ReqInfo {
 public: 
  inline ReqInfo();

 public:
  const ListenAddr* listen_addr;
};

class ProtocolWrite {
 public:
  static const Protocol::Category kCategory = Protocol::kInvalid;
  static const int kEnd = -100;
  
 public:
  virtual Protocol::Category GetCategory() const { return kCategory; }

  virtual void Reset(const Buf& /*buf*/) { return; }
  virtual bool Encode() { return false; }

  /*
   * @return: 
   *     >    0 : there are bytes to be written
   *    ==    0 : all has been written 
   *     <    0 : error happen
   *    == kEnd : 0 bytes are written
   */
  virtual int Write(int /*fd*/) { return -1; }

  virtual ~ProtocolWrite() {}
};

class ProtocolRead {
 public:
  static const Protocol::Category kCategory = Protocol::kInvalid;
  static const int kEnd = -100;
  
 public:
  virtual Protocol::Category GetCategory() const { return kCategory; }

  virtual void Reset(const ListenAddr* listen_addr=NULL);
  virtual bool Decode() { return false; }

  /*
   * @return: 
   *     >    0 : there are bytes to be read
   *    ==    0 : all has been read 
   *     <    0 : error happen
   *    == kEnd : 0 bytes are read
   */
  virtual int Read(int /*fd*/) { return kEnd; }

  const ReqInfo& GetReqInfo() const { return req_info_; }
  const std::string& GetServiceName() const { return req_info_.listen_addr->name; }
  virtual const char* Data() const { return NULL; }
  virtual size_t Size() const { return 0; }

  virtual ~ProtocolRead() {}
 
 public:
  ReqInfo req_info_;
};

bool Addr::Assign(const std::string& addr_str) {
  size_t pos_sep = addr_str.find(':');
  if (std::string::npos == pos_sep) {
    return false;
  }

  std::string ip = addr_str.substr(0, pos_sep);
  std::string port = addr_str.substr(pos_sep+1);

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  if (INADDR_NONE == addr.sin_addr.s_addr) {
    return false;
  }
  addr.sin_port = htons(atoi(port.c_str()));
  return true;
}

ReqInfo::ReqInfo() :
  listen_addr(NULL) {} 

}}
