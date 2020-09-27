#ifndef PUPPET_TYPES_HPP
#define PUPPET_TYPES_HPP

#include <stdint.h>
#include <mpir.h>

#include <vector>
#include <string>
using std::string;
using std::vector;

/*
 *  Basic data types in Puppet. 
 */

enum PUPPET_TYPES {
  TYPE_BIGINT, TYPE_FLOAT, TYPE_STRING,
  TYPE_LIST, TYPE_OBJECT
};

struct PuppetData;

struct PuppetBigInt {
  mpz_t bignum;

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

/*
 *  Types for OS-specific things 
 *  like processes and pipes that
 *  Puppet needs.
 */

struct PuppetProcess {
  int pid;

  PuppetProcess(const char* cmd_line);
};

struct PuppetPipedProcess {
  int pid;
  vector<char> output;

  PuppetPipedProcess(const char* cmd_line);
};


/*
 *  Data types & functions for 
 *  lexer & parser in Puppet
 */

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
