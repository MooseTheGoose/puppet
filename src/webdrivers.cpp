#include "webdrivers.hpp"
#include <stdio.h>

/*
 *  Similarly to webdrivers.hpp, don't mind this file. 
 */

int ChromePuppet::init() {
  int status;
  int portno = 9515;
  struct addrinfo *result, *p, hints;

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  status = getaddrinfo("localhost", std::to_string(portno).data(), &hints, &result);

  if(status) {
    return -1;
  }

  for(p = result; p; p = p->ai_next) {
    SOCKET sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

    if(sock == INVALID_SOCKET) {
      continue;
    }

    status = connect(sock, p->ai_addr, (int)p->ai_addrlen);
    if(status == SOCKET_ERROR) {
      closesocket(sock);
      continue;
    }

    this->puppet_socket = sock;
    this->addr_len = p->ai_addrlen;
    *((struct sockaddr *)&this->puppet_addr) = *p->ai_addr;
    break;
  }

  if(result) {
    freeaddrinfo(result);
  }
  if(!p) {
    return -1;
  }

  return 0;
}