#ifndef BASIC_WEBDRIVERS_HPP
#define BASIC_WEBDRIVERS_HPP

/*
 *  Really basic webdriver which isn't very configurable nor rigorous 
 *  and has no involvement with the Puppet language, but which I can get 
 *  a prototype off the ground really quickly to test my WebDriver
 *  protocol skills. 
 */

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


struct BasicHeaders {
  vector<string> labels;
  vector<string> values;

  BasicHeaders();
  void ManipulateHeader(const string &label, const string &value);
  void DeleteHeader(const string &label);
};

class BasicWebdriver {
  struct sockaddr_storage driver_addr;
  int addr_len;

  public:
    BasicWebdriver();
    vector<char> fetch(const string &method, const string &endpoint, const BasicHeaders &headers, const vector<char> &body);
    vector<char> get(const string &endpoint, const BasicHeaders &headers, const vector<char> &body);
    vector<char> post(const string &endpoint, const BasicHeaders &headers, const vector<char> &body);
    vector<char> del(const string &endpoint, const BasicHeaders &headers, const vector<char> &body);
};


#endif