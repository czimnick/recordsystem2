// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_conn.h"
#include "google/protobuf/rpc/rpc_env.h"

#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef NI_MAXSERV
# define NI_MAXSERV 32
#endif

// MSG_NOSIGNAL does not exists on OS X
#if defined(__APPLE__) || defined(__MACH__)
# ifndef MSG_NOSIGNAL
#   define MSG_NOSIGNAL SO_NOSIGPIPE
# endif
#endif

namespace google {
namespace protobuf {
namespace rpc {

// [static]
// Initialize socket services
bool InitSocket() {
  return true;
}

bool Conn::IsValid() const {
  return sock_ != 0;
}

bool Conn::DialTCP(const char* host, int port) {
  struct sockaddr_in sa;
  int status, len;

  if(IsValid()) Close();
  if((sock_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    logf("socket failed.\n");
    sock_ = 0;
    return false;
  }

  memset(sa.sin_zero, 0 , sizeof(sa.sin_zero));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = inet_addr(host);
  int addressSize = sizeof(sa);

  if(connect(sock_, ( struct sockaddr * )&sa, addressSize ) == -1 ) {
    logf("connect failed.\n");
    Close();
    return false;
  }

  int flag = 1;
  setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

  if(!DoHandshake()) {
    logf("connect failed (handshake).\n");
    Close();
    return false;
  }

  return true;
}

bool Conn::ListenTCP(int port, int backlog) {
  if(IsValid()) Close();
  if((sock_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    logf("socket failed.\n");
    sock_ = 0;
    return false;
  }

  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons((u_short) port);

  if(bind(sock_, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
    logf("bind failed.\n");
    Close();
    return false;
  }
  if(::listen(sock_, backlog) != 0) {
    logf("listen failed.\n");
    Close();
    return false;
  }

  int flag = 1;
  setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
  return true;
}

void Conn::Close() {
  if(IsValid()) {
    ::close(sock_);
    sock_ = 0;
  }
}

Conn* Conn::Accept() {
  struct sockaddr_in *addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  socklen_t addrlen = sizeof(struct sockaddr_in);
  int sock = ::accept(sock_, (struct sockaddr*)addr, &addrlen);
  if(sock == 0) {
    free(addr);
    logf("listen failed.\n");
    return NULL;
  }
  return new Conn(sock, (struct sockaddr*)addr, env_);
}

Conn* Conn::AcceptNonBlock() {
  struct timeval timeout;
  fd_set active_fd_set, read_fd_set;
  struct sockaddr_in *addr = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));
  socklen_t addrlen = sizeof(struct sockaddr_in);

  FD_ZERO(&active_fd_set);
  FD_SET(sock_, &active_fd_set);

  read_fd_set = active_fd_set;

  memset(&timeout, 0, sizeof(timeout));

  timeout.tv_sec = 0;
  timeout.tv_usec = 10;

  int ret = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout);
  if(ret > 0) {
	  int sock = ::accept(sock_, (struct sockaddr*)addr, &addrlen);

	  if(sock == 0) {
        free(addr);
		logf("listen failed.\n");
		return NULL;
	  }

	  return new Conn(sock, (struct sockaddr*)addr, env_);
  }else if (ret < 0) {
	  free(addr);
  }

  return NULL;
}

bool Conn::Read (void* buf, int len) {
  char *cbuf = (char*)buf;
  while(len > 0) {
    int sent = recv(sock_, cbuf, len, 0);
    if(sent == 0 || sent == -1) {
      logf("IO error, err = %d.\n", errno);
      return false;
    }
    cbuf += sent;
    len -= sent;
  }
  return true;
}
bool Conn::Write(void* buf, int len) {
  const char *cbuf = (char*)buf;
  int flags = MSG_NOSIGNAL;

  while(len > 0) {
    int sent = send(sock_, cbuf, len, flags );
    if(sent == -1) {
      logf("IO error, err = %d.\n", errno);
      return false;
    }
    cbuf += sent;
    len -= sent;
  }
  return true;
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google
