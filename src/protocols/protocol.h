#pragma once

#include "../public/common.h"

namespace magneto {

typedef bool (*FuncReadChecker)(const char* buf, size_t count);

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

  virtual void Reset(const char* /*buf*/, size_t /*count*/) { return; }
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
  virtual const char* Buf() const { return NULL; }
  virtual size_t Len() const { return 0; }

  virtual ~ProtocolRead() {}
 
 public:
  ReqInfo req_info_;
};

ReqInfo::ReqInfo() :
  listen_addr(NULL) {} 

}
