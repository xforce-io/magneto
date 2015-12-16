#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "common.h"

namespace xforce {

class IOHelper {
 public:
  inline static bool IpToInt(const char* ipstr, uint32_t& ipint);
  static int CreateSocket(bool is_non_block=true); 
  static int Listen(const sockaddr_in& addr, size_t backlog=2048);
  static int Accept(int listen_fd, char* ipbuf=NULL, int* port=NULL);

  /*
   * @return: <fd, whether need event>
   */
  inline static std::pair<int, bool> Connect(const sockaddr_in& addr, bool is_non_block=true);

  inline static bool SetNonBlock(int fd);
  static int WriteNonBlock(int fd, const void* buf, size_t count);
  static int ReadNonBlock(int fd, void* buf, size_t count);
  static int WriteVecNonBlock(int fd, iovec* iov, size_t& num_iov);

  inline static bool CheckConn(int fd);

  inline static void Close(int fd);
};

bool IOHelper::IpToInt(const char* ipstr, uint32_t& ipint) {
  ipint=0;
  const char* cur=ipstr;
  for (size_t i=0; i<4; ++i) {
    int segvalue = atoi(cur);
    if ((segvalue>255) || (segvalue<0)) {
      return false;
    }

    ipint += segvalue * (uint32_t)(1 << ((3-i)*8));
    cur = strchr(cur, '.');
    if (3!=i && NULL==cur) {
      return false;
    }
    cur+=1;
  }
  return true;
}

std::pair<int, bool> IOHelper::Connect(const sockaddr_in& addr, bool is_non_block) {
  int fd = CreateSocket(is_non_block);
  if (fd<=0) return std::make_pair(-1, true);

  int ret = connect(fd, RCAST<const sockaddr*>(&addr), sizeof(addr));
  if (0==ret) {
    return std::make_pair(fd, true);
  } else if (EINPROGRESS==errno) {
    return std::make_pair(fd, false);
  } else if (EINTR==errno) {
    return Connect(addr, is_non_block);
  } else {
    IOHelper::Close(fd);
    return std::make_pair(-1, true);
  }
}

bool IOHelper::SetNonBlock(int fd) {
  int ret = fcntl(fd, F_GETFL, 0);
  if(-1 != ret) {
    ret = fcntl(fd, F_SETFL, ret | O_NONBLOCK);
    if(-1 != ret) return true;
  }
  return false;
}

bool IOHelper::CheckConn(int fd) {
  char buf[1];
  ssize_t ret = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
  return (ret < 0 && (EWOULDBLOCK == errno || EAGAIN == errno)) ? true : false;
}

void IOHelper::Close(int fd) {
  close(fd);
}

}
