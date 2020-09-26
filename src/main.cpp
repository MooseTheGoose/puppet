#include <string>
#include "puppet_types.hpp"
#include "basic_webdrivers.hpp"

int main(int argc, const char *argv[]) {
  BasicHeaders hdrs = BasicHeaders();
  BasicWebdriver webdrvr = BasicWebdriver();
  int status;
  WSADATA wsaData;

  string bodystr = "{ \"capabilities\" : { \"alwaysMatch\" : { \"browserName\" : \"firefox\"} } }";
  vector<char> body = vector<char>();

  for(int i = 0; i < bodystr.size(); i++) {
    body.push_back(bodystr[i]);
  }

  status = WSAStartup(MAKEWORD(2,2), &wsaData);
  if(status) {
    fprintf(stderr, "WSAStartup failed...\r\n");
    return -1;
  } 

  hdrs.ManipulateHeader("Content-Length", std::to_string(body.size()));
  vector<char> data = webdrvr.fetch("POST", "/session", hdrs, body);
  printf("Connection success\r\n%s\r\n", data.data());
  return 0;
}
