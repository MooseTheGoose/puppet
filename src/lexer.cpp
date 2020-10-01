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

const char *const PUPPET_LINE_SEP = "\n";
const char *const PUPPET_SLCOMMENT = "//";
const char *const PUPPET_MLCOMMENT_OPEN = "/*";
const char *const PUPPET_MLCOMMENT_CLOSE = "*/";

const char *PUPPET_KEYWORDS[] = {
  "SEND", "FIND", "GET", "ALL",
  "EXECUTE", "ASYNC", "CLICK", "URL",
  "TAG", "CSS", "ID", "XPATH", "SLEEP", 
  "TO", 0
};

const char *PUPPET_OPERATORS[] = {
  "-", "+", "=", 0
};

const char *PUPPET_LBRACKETS[] = {
  "{", "(", "[", 0
};

const char *PUPPET_RBRACKETS[] = {
  "}", ")", "]", 0
};

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
    if(peek >= 0x80) {
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
    nltok.type = TOK_NL;
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
  while(!is_prefix(PUPPET_LINE_SEP, local_data + this->index) && curr_char > 0) {
     this->eatchar();
     curr_char = this->peekchar();
  }
  if(curr_char > 0) {
    this->index += strlen(PUPPET_LINE_SEP);
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
  while(mlcomment_stack && curr_char > 0) {
    if(is_prefix(PUPPET_MLCOMMENT_OPEN, local_data + this->index)) {
      mlcomment_stack++;
      this->index += strlen(PUPPET_MLCOMMENT_OPEN);
    } else if(is_prefix(PUPPET_MLCOMMENT_CLOSE, local_data + this->index)) {
      mlcomment_stack--;
      this->index += strlen(PUPPET_MLCOMMENT_CLOSE);
    } else if(is_prefix(PUPPET_LINE_SEP, local_data + this->index)) {
      this->index += strlen(PUPPET_LINE_SEP);
    } else {
      this->eatchar();
    }
    curr_char = this->peekchar();
  }
  if(mlcomment_stack) {
    printf("LEXER ERROR: Missing %s in multi-line comment\r\n", PUPPET_MLCOMMENT_CLOSE);
    curr_char = -1;
  }
  return curr_char;  
}

unichar_t lexer::lex_identifier() {
  unichar_t curr_char = this->peekchar();
  char *iden_ptr;
  string iden_str;
  lexer_token tok;
  size_t last_index = this->index;

  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;

  while(puppet_isiden(curr_char) || puppet_isdigit(curr_char)) {
	this->eatchar();
	iden_str.append(this->data + last_index, this->index - last_index);
	last_index = this->index;
    curr_char = this->peekchar();
  }

  int keyword_index = find_in_strv(iden_str.data(), PUPPET_KEYWORDS);
  if(keyword_index < 0) {
    iden_ptr = new char[this->index - last_index + 1];
    iden_str.copy(iden_ptr, iden_str.size(), 0);
    iden_ptr[iden_str.size()] = 0;
    tok.type = TOK_IDENTIFIER;
    tok.identifier = iden_ptr;
  } else {
    tok.type = TOK_KEYWORD;
    tok.keyword = keyword_index;
  }

  this->token_queue.push_back(tok);
  return curr_char;
}

unichar_t lexer::lex_operator() {
  int index = find_prefix_in_strv(this->data + this->index, PUPPET_OPERATORS);
  lexer_token tok;
  size_t new_index = this->index + strlen(PUPPET_OPERATORS[index]);

  tok.type = TOK_OPERATOR;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  tok.op = index;
  this->token_queue.push_back(tok);

  while(this->index < new_index) {
    this->eatchar();
  }
  return this->peekchar();
}

unichar_t lexer::lex_number() {
  PuppetBigInt i;
  int base = 10;
  unichar_t add = 0;
  unichar_t curr_char = this->peekchar();
  lexer_token tok;
  i.init(0);

  tok.type = TOK_BIGINT;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;

  if(curr_char == '0') {
    this->eatchar();
    curr_char = this->peekchar();
    int specifier = puppet_toupper(curr_char);
    
    if(specifier == 'X' || specifier == 'B' || specifier == 'O') {
      base = (specifier == 'X') ? 16 : ((specifier == 'O') ? 8 : 2);
      this->eatchar();
      curr_char = this->eatchar();
      add = puppet_conv_xdigit(curr_char);
      if(add < 0 || add >= base) {
        i.free();
        printf("LEXER ERROR: digit outside of range (Code: %X)\r\n", add);
        return -1;
      }
      i.addint(add);
      curr_char = this->peekchar();
    } else if(puppet_isdigit(curr_char)) {
      base = 8;
    } else {
      printf("LEXER ERROR: invalid base specifier (Code: %X)\r\n", curr_char);
      i.free();
      return -1;
    }
  }

  while((puppet_isiden(curr_char) || puppet_isdigit(curr_char) || 
         curr_char == '_') && curr_char > 0) {
    while(curr_char == '_') {
      this->eatchar();
      curr_char = this->peekchar();
    }
    add = puppet_conv_xdigit(curr_char);
    if(add < 0 || add >= base) {
      i.free();
      printf("LEXER ERROR: digit outside of range (Code: %X)\r\n", add);      
      return -1;
    }
      
    i.multint(base);
    i.addint(add);
    this->eatchar();
    curr_char = this->peekchar();
  }

  tok.bigint = i;
  this->token_queue.push_back(tok);

  return curr_char;
}

unichar_t lexer::lex_string() {
  lexer_token tok;
  unichar_t quote;
  unichar_t curr_char;
  char *literal_ptr;
  string str;
  size_t last_index;

  tok.type = TOK_STRING;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;

  quote = this->eatchar();
  last_index = this->index;
  curr_char = this->eatchar();

  while(curr_char != quote && curr_char > 0) {
    str.append(this->data + last_index, this->index - last_index);
    last_index = this->index;
    curr_char = this->eatchar();
  }

  if(curr_char <= 0) {
    printf("LEXER ERROR: abrupt end to string (Code: %X)\r\n", curr_char);
  } else {
    literal_ptr = new char[str.size() + 1];
    str.copy(literal_ptr, str.size(), 0);
    literal_ptr[str.size()] = 0;
    tok.strliteral = literal_ptr;
    this->token_queue.push_back(tok);
  }

  return this->peekchar();
}

unichar_t lexer::lex_lbracket() {
  int lbracket_index = find_prefix_in_strv(this->data + this->index, PUPPET_LBRACKETS);
  lexer_token tok;
  size_t new_index = this->index + strlen(PUPPET_LBRACKETS[lbracket_index]);

  tok.type = TOK_LBRACKET;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  tok.brack_type = lbracket_index;
  this->token_queue.push_back(tok);

  while(this->index < new_index) {
    this->eatchar();
  }
  return this->peekchar();
}

unichar_t lexer::lex_rbracket() {
  int rbracket_index = find_prefix_in_strv(this->data + this->index, PUPPET_RBRACKETS);
  lexer_token tok;
  size_t new_index = this->index + strlen(PUPPET_RBRACKETS[rbracket_index]);

  tok.type = TOK_RBRACKET;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  tok.brack_type = rbracket_index;
  this->token_queue.push_back(tok);

  while(this->index < new_index) {
    this->eatchar();
  }
  return this->peekchar();
}

unichar_t lexer::lex_semicolon() {
  lexer_token tok;

  tok.type = TOK_SEMICOLON;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  this->token_queue.push_back(tok);

  this->eatchar();
  return this->peekchar();  
}

int lexer::lex_stage1() {
  unichar_t curr_char = this->peekchar();
  while(curr_char > 0) {
    while(curr_char > 0 && puppet_isspace(curr_char)) {
      this->eatchar();
      curr_char = this->peekchar();
    }

    if(puppet_isdigit(curr_char)) {
      curr_char = this->lex_number();
    } else if(puppet_isiden(curr_char)) {
      curr_char = this->lex_identifier();
    } else if(curr_char == '"' || curr_char == '\'') {
      curr_char = this->lex_string();
    } else if(find_prefix_in_strv(this->data + this->index, PUPPET_OPERATORS) >= 0) {
      curr_char = this->lex_operator();
    } else if(is_prefix(PUPPET_SLCOMMENT, this->data + this->index)) {
      curr_char = this->lex_slcomment();
    } else if(is_prefix(PUPPET_MLCOMMENT_OPEN, this->data + this->index)) {
      curr_char = this->lex_mlcomment();
    } else if(find_prefix_in_strv(this->data + this->index, PUPPET_LBRACKETS) >= 0) {
      curr_char = this->lex_lbracket();
    } else if(find_prefix_in_strv(this->data + this->index, PUPPET_RBRACKETS) >= 0) {
      curr_char = this->lex_rbracket();
    } else if(this->data[this->index] == ';') {
      curr_char = this->lex_semicolon();
    } else if(curr_char) {

      /* Unrecognizable character */
      fprintf(stderr, "LEXER ERROR: Invalid token prefix (code: %X)\r\n", curr_char);
      return -1;
    }

    if(curr_char < 0) {
      return -1;
    }
  }

  lexer_token term;
  term.type = TOK_TERM;
  term.lino = this->curr_lino;
  term.chno = this->curr_chno;
  this->token_queue.push_back(term);
  return 0;
}

int lexer::lex() {
  int status = this->lex_stage1();
  return status;
}

void lexer::init(const char *source) {
	this->curr_lino = 1;
	this->curr_chno = 1;
	this->data = source;
	this->index = 0;
	this->len = strlen(source);
	this->token_queue = vector<lexer_token>();
}