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
#ifdef MAGNETO_PROTOBUF_SUPPORT
    kProtobuf,
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
  static const int kEnd = -10000;
  
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
  static const int kEnd = -10001;
  
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
  virtual int Read(int /*fd*/) { DEBUG("here??"); return kEnd; }

  const ReqInfo& GetReqInfo() const { return req_info_; }
  const std::string& GetServiceName() const { return req_info_.listen_addr->name; }
  virtual const char* Data() const { return NULL; }
  virtual size_t Size() const { return 0; }

  virtual ~ProtocolRead() {}
 
 public:
  ReqInfo req_info_;
};

template <typename TypeHeader>
class ProtocolReadWithFixSize : public ProtocolRead {
 private:
  typedef ProtocolRead Super;
  
 public:
  virtual void Reset(const ListenAddr* listen_addr);
  virtual int Read(int fd);
  virtual const char* Data() const { return Body(); }
  virtual size_t Size() const { return SizeBody(); }

 protected: 
  size_t SizeHeader() const { return sizeof(TypeHeader); }
  virtual size_t SizeBody() const = 0;
  const TypeHeader& Header() const { return *RCAST<const TypeHeader*>(buffer_.Start()); }
  const char* Body() const { return RCAST<const char*>(buffer_.Start() + SizeHeader()); }
  char* Body() { return RCAST<char*>(buffer_.Start() + SizeHeader()); }

 protected:
  Buffer buffer_;
  bool read_header_;
};

template <typename TypeHeader>
void ProtocolReadWithFixSize<TypeHeader>::Reset(const ListenAddr* listen_addr) {
  Super::Reset(listen_addr);

  buffer_.Clear();
  buffer_.Reserve(SizeHeader());
  read_header_=true;
}

template <typename TypeHeader>
int ProtocolReadWithFixSize<TypeHeader>::Read(int fd) {
  bool has_bytes_read=false;
  for (;;) {
    size_t bytes_left;
    if (read_header_) {
      bytes_left = SizeHeader() - buffer_.Len();
      int ret = IOHelper::ReadNonBlock(fd, buffer_.Stop(), bytes_left);
      if (ret>0) {
        has_bytes_read=true;
        buffer_.SetLen(buffer_.Len() + ret);
        if ( SizeHeader() == buffer_.Len() ) {
          read_header_=false;
          buffer_.Reserve(SizeHeader() + SizeBody());
          continue;
        }
      } else if (0==ret) {
        return has_bytes_read ? bytes_left : kEnd;
      } else {
        return ret;
      }
    } else {
      bytes_left = SizeHeader() + SizeBody() - buffer_.Len();
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
