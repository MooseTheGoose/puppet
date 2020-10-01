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
  this->type = PUPPET_STR8;
  this->str8 = new string;
}

void PuppetString::append_unichar(unichar_t curr_char) {
  if(curr_char < 0 || curr_char >= 0x110000) {
    printf("STRING ERROR: Got a unichar_t outside of Unicode range! (Code: %d)\r\n", curr_char);
  } else {
    switch(this->type) {
      case PUPPET_STR8:
        if(curr_char < 0x100) {
          this->str8->push_back(curr_char);
        } else if(curr_char < 0x10000) {
          this->type = PUPPET_STR16;
          u16string *newstr = new u16string;
          for(size_t i = 0; i < this->str8->size(); i++) {
            newstr->push_back(this->str8->data()[i]);
          }
          newstr->push_back(curr_char);
          delete this->str8;
          this->str16 = newstr;
        } else {
          this->type = PUPPET_STR32;
          u32string *newstr = new u32string;
          for(size_t i = 0; i < this->str8->size(); i++) {
            newstr->push_back(this->str8->data()[i]);
          }
          newstr->push_back(curr_char);
          delete this->str8;
          this->str32 = newstr;
        }
        break;
      case PUPPET_STR16:
        if(curr_char < 0x10000) {
          this->str16->push_back(curr_char);
        } else {
          this->type = PUPPET_STR32;
          u32string *newstr = new u32string;
          for(size_t i = 0; i < this->str16->size(); i++) {
            newstr->push_back(this->str16->data()[i]);
          }
          newstr->push_back(curr_char);
          delete this->str16;
          this->str32 = newstr;
        }
        break;
      case PUPPET_STR32:
        this->str32->push_back(curr_char);
        break;
    }
  }
}

string PuppetString::to_string() {
  string retstr = "";

  switch(this->type) {
    case PUPPET_STR8:
      for(size_t i = 0; i < this->str8->size(); i++) {
        if(this->str8->data()[i] == '"') {
          retstr.push_back('\\');
        }
        retstr.push_back(this->str8->data()[i]);
      }
      break;
    case PUPPET_STR16:
      for(size_t i = 0; i < this->str16->size(); i++) {
        unichar_t curr_char = this->str16->data()[i];
        if(curr_char < 0x80) {
          retstr.push_back(curr_char);
        } else if(curr_char >= 0x80 && curr_char < 0x7FF) {
          retstr.push_back(0xC0 | curr_char >> 6 & 0x1F);
          retstr.push_back(curr_char & 0x3F);
        } else {
          retstr.push_back(0xE0 | curr_char >> 12 & 0xF);
          retstr.push_back(0x80 | curr_char >> 6 & 0x3F);
          retstr.push_back(0x80 | curr_char & 0x3F);
        }
      }
      break;
    case PUPPET_STR32:
      for(size_t i = 0; i < this->str32->size(); i++) {
        unichar_t curr_char = this->str32->data()[i];
        if(curr_char < 0x80) {
          retstr.push_back(curr_char);
        } else if(curr_char >= 0x80 && curr_char < 0x7FF) {
          retstr.push_back(0xC0 | curr_char >> 6 & 0x1F);
          retstr.push_back(curr_char & 0x3F);
        } else if(curr_char >= 0x800 && curr_char < 0x10000) {
          retstr.push_back(0xE0 | curr_char >> 12 & 0xF);
          retstr.push_back(0x80 | curr_char >> 6 & 0x3F);
          retstr.push_back(0x80 | curr_char & 0x3F);
        } else if(curr_char >= 0x10000 && curr_char < 0x110000) {
          retstr.push_back(0xF0 | curr_char >> 18 & 0x7);
          retstr.push_back(0x80 | curr_char >> 12 & 0x3F);
          retstr.push_back(0x80 | curr_char >> 6 & 0x3F);
          retstr.push_back(0x80 | curr_char & 0x3F);
        } else {
          /* 
           * TODO: This is an assert. Treat it as such later.
           */
          printf("STRING ERROR: Got a unichar_t outside of Unicode range! (Code: %d)\r\n", curr_char);
        }
      }      
  }
  return "\"" + retstr + "\""; 
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


