#ifndef PUPPET_TYPES_H
#define PUPPET_TYEPS_H

#include <stdint.h>
#include <string>
using std::string;

typedef int32_t unichar_t;
extern const string PUPPET_LINE_SEP;

inline int is_prefix(const char *pre, const char *str) {
  while(*str == *pre && *str && *pre) {
    str++;
    pre++;
  }

  return !*pre;
}


#endif