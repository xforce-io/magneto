#include "../io_helper.h"

namespace magneto {

int IOHelper::CreateSocket(bool is_non_block) {
  int ret, on;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  MAG_FAIL_HANDLE(fd<=0 && (ret=-1))

  if (is_non_block) {
    ret = SetNonBlock(fd);
    MAG_FAIL_HANDLE_AND_SET(true!=ret, ret=-2)
  }

  on=1;
  ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  MAG_FAIL_HANDLE_AND_SET(0!=ret, ret=-3)
  return fd;

  ERROR_HANDLE:
  return ret;
}

int IOHelper::Listen(const sockaddr_in& addr, size_t backlog) {
  int ret;
  int fd = CreateSocket();
  MAG_FAIL_HANDLE_AND_SET(fd<=0, ret=-1)

  ret = bind(fd, RCAST<const sockaddr*>(&addr), sizeof(addr)); 
  MAG_FAIL_HANDLE_AND_SET(0!=ret, ret=-2)

  ret = listen(fd, backlog);
  MAG_FAIL_HANDLE_AND_SET(0!=ret, ret=-3)
  return fd;

  ERROR_HANDLE:
  if (fd>0) IOHelper::Close(fd);
  return ret;
}

int IOHelper::Accept(int listen_fd, char* ipbuf, int* port) {
  sockaddr_in addr;
  socklen_t len_addr = sizeof(addr);
  int new_fd;
  while (true) {
    new_fd = accept(listen_fd, RCAST<sockaddr*>(&addr), &len_addr);
    if (-1==new_fd) {
      if (EINTR==errno) continue;
      return -1;
    }
    break;
  }

  bool ret = SetNonBlock(new_fd);
  if (!ret) return -2;

  if (NULL!=ipbuf) {
    inet_ntop(AF_INET, &addr.sin_addr, ipbuf, INET_ADDRSTRLEN);
  }

  if (NULL!=port) {
    *port = ntohs(addr.sin_port);
  }
  return new_fd;
}

int IOHelper::WriteNonBlock(int fd, const void* buf, size_t count) {
  size_t bytes_write=0;
  while (true) {
    ssize_t tmp_cnt = write(fd, RCAST<const char*>(buf) + bytes_write, count-bytes_write);
    if (tmp_cnt>0) {
      bytes_write+=tmp_cnt;
      if (bytes_write==count) {
        return count;
      } else if (bytes_write>count) {
        return -1;
      }
    } else if (tmp_cnt<0) {
      if (EAGAIN==errno || EWOULDBLOCK==errno || EINTR==errno) {
        return bytes_write;
      } else {
        return -2;
      }
    } else {
      return bytes_write;
    }
  }
}

int IOHelper::ReadNonBlock(int fd, void* buf, size_t count) {
  size_t bytes_read=0;
  while (true) {
    ssize_t tmp_cnt = read(fd, RCAST<char*>(buf) + bytes_read, count-bytes_read);
    if (tmp_cnt>0) {
      bytes_read+=tmp_cnt;
      if (bytes_read==count) {
        return count;
      } else if (bytes_read>count) {
        return -1;
      }
    } else if (tmp_cnt<0) {
      if (EAGAIN==errno || EWOULDBLOCK==errno || EINTR==errno) {
        return bytes_read;
      } else {
        return -2;
      }
    } else {
      return bytes_read;
    }
  }
}

int IOHelper::WriteVec2NonBlock(int fd, const iovec* iov) {
  iovec tmp_iov[2];
  tmp_iov[0] = iov[0];
  tmp_iov[1] = iov[1];

  size_t current_iov_index=0;
  size_t bytes_write=0;
  while (true) {
    ssize_t tmp_cnt = writev(fd, tmp_iov+current_iov_index, 2-current_iov_index);
    if (tmp_cnt>0) {
      bytes_write+=tmp_cnt;
      if (bytes_write == tmp_iov[0].iov_len + tmp_iov[1].iov_len) {
        return bytes_write;
      } else if (bytes_write >= tmp_iov[0].iov_len) {
        current_iov_index=1;
        tmp_iov[1].iov_base = RCAST<char*>(iov[1].iov_base) + bytes_write - iov[0].iov_len; 
        tmp_iov[1].iov_len = iov[0].iov_len + iov[1].iov_len - bytes_write;
      } else {
        tmp_iov[0].iov_base = RCAST<char*>(iov[0].iov_base) + bytes_write; 
        tmp_iov[0].iov_len = iov[0].iov_len - bytes_write;
      }
    } else if (tmp_cnt<0) {
      if (EAGAIN==errno || EWOULDBLOCK==errno || EINTR==errno) {
        return bytes_write;
      } else {
        return -2;
      }
    } else {
      return bytes_write;
    }
  }
}


}
