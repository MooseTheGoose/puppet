#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using std::vector;
using std::string;

#define PORT_NO 4444

BOOL start_webdriver() {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char cmd[256];

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  sprintf(cmd, "geckodriver --port %d", PORT_NO); 

  return CreateProcessA(0, cmd, 0, 0, 0, CREATE_NO_WINDOW, 0, 0, &si, &pi);
}

/*
 *  Small demo demonstrating my extreme lack of HTTP prowess.
 */
int main() {
  int status = start_webdriver();
  int clisock;
  struct addrinfo *result = 0, *p = 0, hints;
  WSADATA wsaData;
  char buffer[9192];

  if(!status) { fprintf(stderr, "Failed to start webdriver %d\r\n", GetLastError()); return -1; }
  status = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if(status) {
    fprintf(stderr, "WSAStartup failed: %d\r\n", status);
    return -1;
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  sprintf(buffer, "%d", PORT_NO);
  status = getaddrinfo("127.0.0.1", buffer, &hints, &result);

  if(status) { fprintf(stderr, "getaddrinfo failed\r\n"); return -1; }
 
  for(p = result; p; p = p->ai_next) {
    clisock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(clisock == INVALID_SOCKET) {
      fprintf(stderr, "Socket creation failed\r\n");
      continue;
    }

    status = connect(clisock, p->ai_addr, (int)p->ai_addrlen);
    if(status == SOCKET_ERROR) {
      fprintf(stderr, "Socket connect failed\r\n");
      continue;
    }

    break;
  }

  freeaddrinfo(result);
  if(!p) {
    fprintf(stderr, "gai failed\r\n");
    return -1;
  }

  string s = "GET /status HTTP/1.1\r\n"
             "Connection: close\r\n"
             "\r\n";
  status = send(clisock, s.data(), s.size(), 0);
  if(status == SOCKET_ERROR) {
    fprintf(stderr, "send failed...\r\n");
    return -1;
  }

  status = recv(clisock, buffer, 9192, 0);
  if(status <= 0) {
    fprintf(stderr, "recv failed...\r\n");
    return -1;
  }

  printf("%d\r\n", status);
  buffer[status] = 0;
  printf("%s\r\n", buffer);
  status = recv(clisock, buffer, 9192, 0);
  buffer[status] = 0;
  printf("%s\r\n", buffer);
  return 0;  
}