#include "puppet_types.hpp"
#include <string.h>
#include <stdio.h>
#include <cstdlib>

int is_prefix(const char *pre, const char *str) {
  while(*str == *pre && *str && *pre) {
    str++;
    pre++;
  }

  return !*pre;
}

int find_prefix_in_strv(const char *str, const char **strv) {
  int index = 0;
  const char *prefix = *strv;

  while(prefix && !is_prefix(prefix, str)) {
    index++;
    prefix = strv[index];
  }
  if(!prefix) {
    index = -1;
  }

  return index;
}

int find_in_strv(const char *str, const char **strv) {
  int index = 0;
  const char *compare = *strv;

  while(compare && strcmp(compare, str)) {
    index++;
    compare = strv[index];
  }
  if(!compare) {
    index = -1;
  }

  return index;
}


int puppet_isspace(unichar_t ch) {
  int yes = 0;

  static uint16_t spaces[] = {
    /* ASCII Whitespace, non-contiguous */
    ' ',

    /* 
     * Unicode space separators not in ASCII
     * and not contiguous
     */
    0xA0, 0x1680, 0x202F, 0x205F, 0x3000,

    /* 
     * Paragraph and "line separator" 
     * (Unicode's line separator, not Foundations's) 
     */
    0x2028, 0x2029,

    /* Null terminator */
    0
  };

  if(ch >= '\t' && ch <= '\r' || ch >= 0x2000 && ch <= 0x2010) { 
    yes = 1; 
  } else {
    for(int i = 0; spaces[i] && !yes; i++) {
      if(ch == spaces[i]) { 
        yes = 1; 
      } 
    }
  }

  return yes;
}

unichar_t puppet_toupper(unichar_t ch) {
  if(ch >= 'a' && ch <= 'z') {
    ch += 'A' - 'a';
  }
  return ch;
}

unichar_t puppet_tolower(unichar_t ch) {
  if(ch >= 'A' && ch <= 'Z') {
    ch += 'a' - 'A';
  }
  return ch;
}

unichar_t puppet_conv_xdigit(unichar_t ch) {
  ch = puppet_toupper(ch);
  return ch >= 'A' ? (ch - 'A' + 10) : (ch - '0');
}

int puppet_isdigit(unichar_t ch) {
  return ch >= '0' && ch <= '9';
}

int puppet_isalpha(unichar_t ch) {
  ch = puppet_toupper(ch);
  return ch >= 'A' && ch <= 'Z';
}

int puppet_isiden(unichar_t ch) {
  return puppet_isalpha(ch) || ch == '_' || ch >= 0x80 && !puppet_isspace(ch);
}

int puppet_isxdigit(unichar_t ch) {
  ch = puppet_conv_xdigit(ch);
  return ch >= 0 && ch < 16;
}

string PuppetBigInt::to_string() {
  char *buffer = mpz_get_str(0, 10, this->bignum);
  string s = buffer;
  std::free(buffer);
   
  return s;
}

int utf8_str::construct_from_bytes(const char *bytestr) {
  string local_str = "";
  size_t local_byte_index = 0;
  size_t local_len = 0;
  
  while(bytestr[local_byte_index]) {
    uint8_t peek = (uint8_t)bytestr[local_byte_index++];
    local_str.push_back(peek);
    if(peek >= 0x80) {
      int n_more_bytes = 0;
      uint8_t prefix = peek << 1;
      unichar_t curr_char;

      while(prefix & 0x80) {
        n_more_bytes++;
        prefix <<= 1;
      }

      if(n_more_bytes < 1 || n_more_bytes > 3) {
        local_str.clear();
        this->byte_index = local_byte_index - 1;
        return -1;
      }

      unichar_t low_bound = 1 << (((n_more_bytes + 3) * (n_more_bytes + 2)) / 2 + 1);
      curr_char = prefix >> (n_more_bytes + 1);
      for(int i = 0; i < n_more_bytes; i++) {
        peek = bytestr[local_byte_index++];
        local_str.push_back(peek);
        if((peek & 0xC0) != 0x80) {
          local_str.clear();
          this->byte_index = local_byte_index - i - 2;
          return -1;
        }
        curr_char <<= 6;
        curr_char |= (peek & 0x3F);
      }
      if(curr_char < low_bound || curr_char > 0x10FFFF) {
        local_str.clear();
        this->byte_index = local_byte_index - n_more_bytes - 1;
        return -1;
      }
    }
    local_len++;
  }
  this->str = local_str; 
  this->len = local_len;
  this->byte_index = 0;
  this->index = 0;
  return 0;
}

unichar_t utf8_str::peekchar() {
  unichar_t peek;
  size_t local_byte_index = this->byte_index;

  if(this->index > this->len) {
    return -1;
  }
  peek = (uint8_t)this->str[local_byte_index++];
  if(peek >= 0x80) {
    int n_more_bytes = 0;
    unsigned char prefix = peek << 1;
    unichar_t curr_char;

    while(prefix & 0x80) {
      n_more_bytes++;
      prefix <<= 1;
    }

    peek = prefix >> (n_more_bytes + 1);
    for(int i = 0; i < n_more_bytes; i++) {
      curr_char = this->str[local_byte_index++];
      peek <<= 6;
      peek |= (curr_char & 0x3F);
    }
  }
  return peek;
}

unichar_t utf8_str::eatchar() {
  unichar_t eat = this->peekchar();

  if(eat > 0) {
    size_t local_byte_index = this->byte_index;
    while((this->str[++local_byte_index] & 0xC0) == 0x80) {}
    this->byte_index = local_byte_index;
    this->index++;
  }
  return eat;
}

unichar_t utf8_str::pukechar() {
  size_t local_byte_index = this->byte_index;
  unichar_t puke = -1;
  if(local_byte_index) {
    while((this->str[--local_byte_index] & 0xC0) == 0x80) {}
    this->byte_index = local_byte_index;
    this->index--;
    puke = this->peekchar();
  }
  return puke;
}

int utf8_str::append_unichar(unichar_t ch) {
  int status = -1;
  char prefix = 0xC0;
  if(ch >= 0 || ch < 0x110000) {
    if(ch < 0x80) {
      this->str.push_back(ch);
    } else {
      int8_t prefix = 0x80;
      while(ch > 0x40) {
        this->str.push_back(ch & 0x3F);
        ch >>= 6;
        prefix >>= 1;
      }
      this->str.push_back(prefix | ch);
    }
    this->len++;
  }
  return status;
}

const char *utf8_str::current() {
  return this->str.data() + this->byte_index;
}

const char *utf8_str::new_literal() {
  char *literal = new char[this->str.size() + 1];
  this->str.copy(literal, this->str.size(), 0);
  literal[this->str.size()] = 0;
  return literal;
}

void PuppetBigInt::init(int initial) {
  mpz_init_set_si(this->bignum, initial);
}

void PuppetBigInt::multint(int mult) {
  mpz_mul_si(this->bignum, this->bignum, mult);
}

void PuppetBigInt::addint(int add) {
  if(add < 0) {
    mpz_sub_ui(this->bignum, this->bignum, ((unsigned int)-add));
  } else {
    mpz_add_ui(this->bignum, this->bignum, add);
  }
}

void PuppetBigInt::subint(int sub) {
  if(sub < 0) {
    mpz_add_ui(this->bignum, this->bignum, ((unsigned int)-sub));
  } else {
    mpz_sub_ui(this->bignum, this->bignum, sub);
  }
}

void PuppetBigInt::divint(int div) {
  PuppetBigInt dividend;
  dividend.init(div);
  mpz_tdiv_q(this->bignum, this->bignum, dividend.bignum);
  dividend.free();
}

void PuppetBigInt::modint(int mod) {
  PuppetBigInt modulus;
  modulus.init(mod);
  mpz_tdiv_r(this->bignum, this->bignum, modulus.bignum);
  modulus.free();
}

void PuppetBigInt::free() {
  mpz_clear(this->bignum);
}

string PuppetFloat::to_string() {
  return std::to_string(this->flt);
}

void PuppetString::init() {
  this->str.construct_from_bytes("");
}

void PuppetString::append_unichar(unichar_t ch) {
  this->str.append_unichar(ch);
}

string PuppetString::to_string() {
  return "\"" + this->str.str + "\""; 
}

string PuppetList::to_string() {
  string buf = "[";

  for(size_t i = 0; i < this->data.size(); i++) {
    PuppetData pd = this->data[i];
    buf += (pd.to_string() + ",");
  }
  if(buf.back() == ',') {
    buf.pop_back();
  }
  buf += "]";

  return buf;
}

string PuppetObject::to_string() {
  string buf = "{";

  for(size_t i = 0; i < this->keys.size(); i++) {
    buf += (this->keys[i].to_string() + ":" + this->values[i].to_string() + ",");
  }
  if(buf.back() == ',') {
    buf.pop_back();
  }
  buf += "}";
 
  return buf;
}

string PuppetData::to_string() {
  string rep = "";

  switch(identifier) {
    case TYPE_BIGINT:
      rep = this->i->to_string();
      break;
    case TYPE_FLOAT:
      rep = this->f->to_string();
      break;
    case TYPE_STRING:
      rep = this->s->to_string();
      break;
    case TYPE_LIST:
      rep = this->arr->to_string();
      break; 
    case TYPE_OBJECT:
      rep = this->obj->to_string();
      break; 
  }

  return rep;
}


