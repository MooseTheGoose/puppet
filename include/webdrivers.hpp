#ifndef WEBDRIVERS_HPP
#define WEBDRIVERS_HPP

#include <string>
#include <vector>
#include <curl/curl.h>

using std::string;
using std::vector;

class GeckoPuppet {
  CURL *session;

  public:
    GeckoPuppet();
    static int GeckoDriver();
};

#endif