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

int IOHelper::WriteVecNonBlock(int fd, iovec* iov, size_t& num_iov) {
  size_t bytes_write=0;
  while (true) {
    ssize_t tmp_cnt = writev(fd, iov, num_iov);
    if (tmp_cnt>0) {
      bytes_write+=tmp_cnt;
      size_t pos_iov;
      for (pos_iov=0; pos_iov<num_iov; ++pos_iov) {
        if (tmp_cnt >= ssize_t(iov[pos_iov].iov_len)) {
          tmp_cnt -= iov[pos_iov].iov_len;
        } else {
          break;
        }
      }

      num_iov=num_iov-pos_iov;
      if (0==num_iov) {
        return bytes_write;
      } else {
        memcpy(iov, iov+pos_iov, num_iov * sizeof(iovec));
        iov[pos_iov].iov_base = RCAST<char*>(iov[pos_iov].iov_base) + tmp_cnt;
        iov[pos_iov].iov_len -= tmp_cnt;
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
