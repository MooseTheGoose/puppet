#ifndef WEBDRIVERS_HPP
#define WEBDRIVERS_HPP

#include <string>
#include <vector>
#include <curl/curl.h>
#include "puppet_types.hpp"

using std::string;
using std::vector;

struct GeckoPuppet {
  CURL *session;
  const char *local_addr;
  PuppetProcess driver;

  int init();
  int quit();
};

#endif