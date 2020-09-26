#ifndef WEBDRIVERS_HPP
#define WEBDRIVERS_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

/* 
 * Don't mind this file for now.
 * It's just puke off the top of my head...
 */

class ChromePuppet {
  struct sockaddr_storage puppet_addr;
  int addr_len;
  SOCKET puppet_socket;

  public:
    int init();
};

#endif