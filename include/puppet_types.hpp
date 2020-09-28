#ifndef PUPPET_TYPES_HPP
#define PUPPET_TYPES_HPP

#include <stdint.h>
#include <mpir.h>

#include <vector>
#include <string>
using std::string;
using std::vector;

#if defined(_WIN32)
#include <windows.h>
#endif

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
 *  Puppet needs. Certainly not for
 *  general use, but suits Puppet's needs
 *  just fine.
 */

/*
 *  Represents background processes which
 *  runs paralell to this process.
 */
struct PuppetProcess {
  int pid;

  #if defined(_WIN32)
  HANDLE hproc;
  #endif

  int init(const char *cmd_line);
  void wait();
  int murder();
};

/*
 *  Piped process which captures stdout & stderr and waits
 *  until that is done.
 */
struct PuppetPipedProcess {
  PuppetProcess process;
  const char *output;
  size_t len;

  int init(const char *cmd_line);
  void wait();
  int murder();
  void free_output();
};

/*
 *  Data types & functions for 
 *  lexer & parser in Puppet
 */

typedef int32_t unichar_t;
extern const string PUPPET_LINE_SEP;

static inline int is_prefix(const char *pre, const char *str) {
  while(*str == *pre && *str && *pre) {
    str++;
    pre++;
  }

  return !*pre;
}

#endif
