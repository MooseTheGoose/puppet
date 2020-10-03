#ifndef PUPPET_TYPES_HPP
#define PUPPET_TYPES_HPP

#include <stdint.h>
#include <mpir.h>

#include <vector>
#include <string>
using std::string;
using std::vector;
using std::u16string;
using std::u32string;

/*
 *  Data types & functions for 
 *  lexer & parser in Puppet
 */

typedef int32_t unichar_t;
struct Utf8String {
  string str;
  size_t byte_index;
  size_t index;
  size_t len;

  int append_unichar(unichar_t ch);
  int construct_from_bytes(const char *bytestr);
  unichar_t peekchar();
  unichar_t eatchar();
  unichar_t pukechar();
  void end();
  void begin();
  const char *begin_bytes();
  const char *current();
  const char *new_literal();
};

int is_prefix(const char *pre, const char *str);
int find_prefix_in_strv(const char *str, const char **strv);
int find_in_strv(const char *str, const char **strv);
int puppet_isspace(unichar_t ch);
int puppet_isdigit(unichar_t ch);
int puppet_isalpha(unichar_t ch);
int puppet_isiden(unichar_t ch);
int puppet_isxdigit(unichar_t ch);
unichar_t puppet_conv_xdigit(unichar_t ch);
unichar_t puppet_toupper(unichar_t ch);
unichar_t puppet_tolower(unichar_t ch);


/*
 *  Basic data types in Puppet. 
 */

enum PUPPET_TYPES {
  TYPE_BIGINT, TYPE_FLOAT, TYPE_STRING,
  TYPE_LIST, TYPE_OBJECT
};

struct PuppetData;
struct PuppetFloat;
struct PuppetBigInt;
struct PuppetString;
struct PuppetList;
struct PuppetObject;

struct PuppetBigInt {
  mpz_t bignum;

  string to_string();
  void init(int initial);
  void multint(int mult);
  void divint(int div);
  void modint(int mod);
  void addint(int add);
  void subint(int sub);
  void free();
  PuppetFloat to_float();
};

struct PuppetFloat {
  double flt;

  void init();
  string to_string();
  PuppetBigInt to_bigint();
};

struct PuppetString {
  Utf8String str;

  void init();
  void init_literal(const char *bytes);
  void append_unichar(unichar_t ch);
  int compare(PuppetString &rhs);
  string to_string();
};

struct PuppetList {
  vector<PuppetData> data;

  string to_string();
};

struct PuppetObject {
  vector<PuppetString> keys;
  vector<PuppetData> values;

  void init();
  void add_pair(PuppetString &key, PuppetData &value);
  int contains(PuppetString &key);
  string to_string();
};

struct PuppetData {
  int type;
  union {
    PuppetBigInt *i;
    PuppetFloat *f;
    PuppetString *s;
    PuppetList *arr;
    PuppetObject *obj;
  };

  string to_string();
  void free();
};



#endif
