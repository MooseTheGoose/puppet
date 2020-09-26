#include "lexer.hpp"



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

  if(peek >= 0 && is_prefix(PUPPET_LINE_SEP.data(), this->data + this->index)) {
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