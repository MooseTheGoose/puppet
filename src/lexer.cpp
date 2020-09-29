#include "lexer.hpp"
#include <stdio.h>
#include <stdlib.h>

/*
 *  TODO (Maybe): Consider dedicating a lexer method to eating
 *                escape sequences like \uNNNN and \xNN outside
 *                and inside of strings after you finish a lexer
 *                and parser prototype. There are more than several
 *                compilers doing this.
 */

const char *PUPPET_LINE_SEP = "\n";
const char *PUPPET_SLCOMMENT = "//";
const char *PUPPET_MLCOMMENT_OPEN = "/*";
const char *PUPPET_MLCOMMENT_CLOSE = "*/";

/*
 *  Peek at the next character using UTF-8
 *  encoding to determine character.
 *
 *  If it fails (out-of-bounds, bad encoding, etc.),
 *  return a negative number.
 */
unichar_t lexer::peekchar() {
  unichar_t peek = -1;
  size_t local_len = this->len;
  size_t local_index = this->index;
  unsigned char *local_data = (unsigned char *)this->data;

  if(local_index <= local_len) {
    peek = local_data[local_index++];
    if(peek > 0x80) {
      int n_more_bytes = 0;
      unichar_t cur_char = peek;
      unsigned char prefix = cur_char << 1;

      while(prefix & 0x80) {
        n_more_bytes++; 
        prefix <<= 1;
      }

      if(n_more_bytes < 1 || n_more_bytes > 3) {
        return -1;
      }

      cur_char = prefix >> (n_more_bytes + 1);
      unichar_t low_bound = 0x04;

      for(int i = 0; i < n_more_bytes; i++) {
        if(local_index > local_len) {
          return -1;
        }
        cur_char = local_data[local_index++];
        if((cur_char & 0xC0) != 0x80) {
          return -1;
        }
        peek <<= 6;
        peek |= (cur_char & 0x3F);

        low_bound <<= 5;
      }

      if(peek < low_bound || peek > 0x10FFFF) {
        return -1;
      }
    }
  }

  return peek;
}

/*
 *  Peek at next character, eat it, and
 *  push a token if appropriate.
 */
unichar_t lexer::eatchar() {
  unichar_t peek = this->peekchar();

  if(peek >= 0 && is_prefix(PUPPET_LINE_SEP, this->data + this->index)) {
    lexer_token nltok;
    nltok.identifier = TOK_NL;
    nltok.lino = this->curr_lino;
    nltok.chno = this->curr_chno;

    this->token_queue.push_back(nltok);
    this->curr_lino++;
    this->curr_chno = 0;
  }

  if(peek >= 0 && peek <= 0x7F) {
    this->index += 1;
  } else if(peek >= 0x80 && peek <= 0x7FF) {
    this->index += 2;
  } else if(peek >= 0x800 && peek <= 0xFFFF) {
    this->index += 3;
  } else if(peek >= 0x10000 && peek <= 0x10FFFF) {
    this->index += 4;
  }

  this->curr_chno++;

  return peek;
}

unichar_t lexer::lex_slcomment() {
  const char *local_data = this->data;
  unichar_t curr_char;

  this->index += strlen(PUPPET_SLCOMMENT);
  curr_char = this->peekchar();
  while(!is_prefix(PUPPET_LINE_SEP, local_data + this->index) && last_char) {
     this->eatchar();
     curr_char = this->peekchar();
  }
  return curr_char;
}

unichar_t lexer::lex_mlcomment() {
  const char *local_data = this->data;
  unichar_t curr_char;
  size_t mlcomment_stack = 1;

  this->index += strlen(PUPPET_MLCOMMENT_OPEN);
  curr_char = this->peekchar();
  while(mlcomment_stack && last_char) {
    if(is_prefix(PUPPET_MLCOMMENT_OPEN, local_data + this->index)) {
      mlcomment_stack++;
      this->index += strlen(PUPPET_MLCOMMENT_OPEN);
    } else if(is_prefix(PUPPET_MLCOMMENT_CLOSE, local_data + this->index) {
      mlcomment_stack--;
      this->index += strlen(PUPPET_MLCOMMENT_CLOSE);
    } else {
      this->eatchar();
    }
    curr_char = this->peekchar();
  }
  if(mlcomment_stack) {
    curr_char = -1;
  }
  return last_char;  
}

int lexer::lex_stage1(const string &source) {
  this->curr_lino = 1;
  this->curr_chno = 1;
  this->data = source.data();
  this->index = 0;
  this->len = source.size();
  this->token_queue = vector<lexer_token>();

  unichar_t curr_char = this->peekchar();
  while(curr_char) {
    while(puppet_isspace(curr_char) && curr_char) {
      this->eatchar();
      curr_char = this->peekchar();
    }

    if(puppet_isdigit(curr_char)) {
      /* Derive the number */
    } else if(puppet_isiden(curr_char)) {
      /* Derive the identifier and/or keyword. */
    } else if(curr_char == '"' || curr_char == '\'') {
      /* Derive the string */
    } else if(/* Check if prefixed by operator token */) {
      /* Derive the operator */
    } else if(is_prefix(PUPPET_SLCOMMENT, this->data + this->index) {
      curr_char = this->lex_slcomment();
    } else if(is_prefix(PUPPET_MLCOMMENT_OPEN, this->data + this->index)) {
      curr_char = this->lex_mlcomment();
    } else {
      /* Unrecognizable character */
      fprintf(stderr, "LEXER ERROR: Invalid token prefix (code: %X)\r\n", curr_char);
      return -1;
    }
  }

  return 0;
}