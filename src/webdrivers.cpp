#include "webdrivers.hpp"
#include <stdio.h>

GeckoPuppet::GeckoPuppet() {
  session = curl_easy_init();
}