#ifndef PUPPET_TYPES_H
#define PUPPET_TYEPS_H

#include <stdint.h>
#include <vector>
#include <string>
using std::string;
using std::vector;

enum PUPPET_TYPES {
  TYPE_BIGINT, TYPE_FLOAT, TYPE_STRING,
  TYPE_LIST, TYPE_OBJECT
};

struct PuppetData;

struct PuppetBigInt {
  /* Fill in eventually with an actual big integer... */
  int num;

  string to_string();
};

struct PuppetFloat {
  double num;

  string to_string();
};

struct PuppetString {
  string str;

  string to_string();
};

struct PuppetList {
  vector<PuppetData> data;

  string to_string();
};

struct PuppetObject {
  vector<PuppetString> keys;
  vector<PuppetData> values;

  string to_string();
};

struct PuppetData {
  int identifier;
  union {
    PuppetBigInt *i;
    PuppetFloat *f;
    PuppetString *s;
    PuppetList *arr;
    PuppetObject *obj;
  };

  string to_string();
};

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