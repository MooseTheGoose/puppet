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
  ":", ",", "-", "+", "=", 0
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
unichar_t Lexer::peekchar() {
  return this->data.peekchar();
}

/*
 *  Peek at next character, eat it, and
 *  push a token if appropriate.
 */
unichar_t Lexer::eatchar() {
  unichar_t eat = this->data.eatchar();

  if(eat >= 0 && is_prefix(PUPPET_LINE_SEP, this->current())) {
    LexerToken nltok;
    nltok.type = TOK_NL;
    nltok.lino = this->curr_lino;
    nltok.chno = this->curr_chno;

    this->token_queue.push_back(nltok);
    this->curr_lino++;
    this->curr_chno = 0;
  }
  this->curr_chno++;

  return eat;
}

unichar_t Lexer::eatstr(const char *str) {
  const char *before = this->current();
  size_t len = strlen(str);
  while(this->current() < before + len) {
    this->eatchar();
  }
  return this->peekchar();
}

const char *Lexer::current() {
  return this->data.current();
}

unichar_t Lexer::lex_slcomment() {
  const char *before = this->current();
  unichar_t curr_char;

  while(this->current() < before + strlen(PUPPET_SLCOMMENT)) {
    this->eatchar();
  }
  curr_char = this->peekchar();
  while(!is_prefix(PUPPET_LINE_SEP, this->current()) && curr_char > 0) {
     this->eatchar();
     curr_char = this->peekchar();
  }
  if(curr_char > 0) {
    curr_char = this->eatstr(PUPPET_LINE_SEP);
    this->token_queue.pop_back();
  }
  return curr_char;
}

unichar_t Lexer::lex_mlcomment() {
  const char *local_data = this->current();
  unichar_t curr_char;
  size_t mlcomment_stack = 1;

  while(this->current() < local_data + strlen(PUPPET_MLCOMMENT_OPEN)) {
    this->eatchar();
  }
  local_data = this->current();
  curr_char = this->peekchar();
  while(mlcomment_stack && curr_char > 0) {
    if(is_prefix(PUPPET_MLCOMMENT_OPEN, local_data)) {
      mlcomment_stack++;
      curr_char = this->eatstr(PUPPET_MLCOMMENT_OPEN);
    } else if(is_prefix(PUPPET_MLCOMMENT_CLOSE, local_data)) {
      mlcomment_stack--;
      curr_char = this->eatstr(PUPPET_MLCOMMENT_CLOSE);
    } else if(is_prefix(PUPPET_LINE_SEP, local_data)) {
      curr_char = this->eatstr(PUPPET_LINE_SEP);
      this->token_queue.pop_back();
    } else {
      this->eatchar();
      curr_char = this->peekchar();
    }
    local_data = this->current();
  }
  if(mlcomment_stack) {
    this->logger.append_line("LEXER ERROR: Missing multi-line comment ending(s)\r\n");
    this->status = -1;
  }
  return curr_char;  
}

unichar_t Lexer::lex_identifier() {
  unichar_t curr_char = this->peekchar();
  Utf8String iden_str;
  LexerToken tok;

  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;

  iden_str.construct_from_bytes("");
  while(puppet_isiden(curr_char) || puppet_isdigit(curr_char)) {
    iden_str.append_unichar(curr_char);
    this->eatchar();
    curr_char = this->peekchar();
  }

  int keyword_index = find_in_strv(iden_str.current(), PUPPET_KEYWORDS);
  if(keyword_index < 0) {
    tok.type = TOK_IDENTIFIER;
    tok.identifier = iden_str.new_literal();
  } else {
    tok.type = TOK_KEYWORD;
    tok.keyword = keyword_index;
  }

  this->token_queue.push_back(tok);
  return curr_char;
}

unichar_t Lexer::lex_operator() {
  const char *before = this->current();
  int index = find_prefix_in_strv(before, PUPPET_OPERATORS);
  LexerToken tok;

  tok.type = TOK_OPERATOR;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  tok.op = index;
  this->token_queue.push_back(tok);

  return this->eatstr(PUPPET_OPERATORS[index]);
}

unichar_t Lexer::lex_number() {
  PuppetBigInt i;
  int base = 10;
  unichar_t add = 0;
  unichar_t curr_char = this->peekchar();
  LexerToken tok;
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
        this->logger.append_line("LEXER ERROR: expected digit after base specifier");
        this->status = -1;
        return this->peekchar();
      }
      i.addint(add);
      curr_char = this->peekchar();
    } else if(puppet_isdigit(curr_char)) {
      base = 8;
    } else {
      this->logger.append_line("LEXER ERROR: invalid base specifier");
      i.free();
      return this->peekchar();
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
      this->logger.append_line("LEXER ERROR: digit outside of range");
      this->status = -1;  
      return this->peekchar();
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

unichar_t Lexer::lex_string() {
  LexerToken tok;
  unichar_t quote;
  unichar_t curr_char;
  Utf8String str;
  size_t last_index;

  tok.type = TOK_STRING;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;

  quote = this->eatchar();
  curr_char = this->eatchar();

  while(curr_char != quote && curr_char > 0) {
    str.append_unichar(curr_char);
    curr_char = this->eatchar();
  }

  if(curr_char <= 0) {
    this->logger.append_line("LEXER ERROR: abrupt end to string");
    this->status = -1;
    return curr_char;
  } else {
    tok.strliteral = str.new_literal();
    this->token_queue.push_back(tok);
  }

  return this->peekchar();
}

unichar_t Lexer::lex_lbracket() {
  int lbracket_index = find_prefix_in_strv(this->current(), PUPPET_LBRACKETS);
  LexerToken tok;
  const char *before = this->current();

  tok.type = TOK_LBRACKET;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  tok.brack_type = lbracket_index;
  this->token_queue.push_back(tok);

  return this->eatstr(PUPPET_LBRACKETS[lbracket_index]);
}

unichar_t Lexer::lex_rbracket() {
  int rbracket_index = find_prefix_in_strv(this->current(), PUPPET_RBRACKETS);
  LexerToken tok;
  const char *before = this->current();

  tok.type = TOK_RBRACKET;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  tok.brack_type = rbracket_index;
  this->token_queue.push_back(tok);

  return this->eatstr(PUPPET_RBRACKETS[rbracket_index]);
}

unichar_t Lexer::lex_semicolon() {
  LexerToken tok;

  tok.type = TOK_SEMICOLON;
  tok.lino = this->curr_lino;
  tok.chno = this->curr_chno;
  this->token_queue.push_back(tok);

  this->eatchar();
  return this->peekchar();  
}

int Lexer::lex_stage1() {
  if(this->status == -1) {
    return -1;
  }

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
    } else if(find_prefix_in_strv(this->current(), PUPPET_OPERATORS) >= 0) {
      curr_char = this->lex_operator();
    } else if(is_prefix(PUPPET_SLCOMMENT, this->current())) {
      curr_char = this->lex_slcomment();
    } else if(is_prefix(PUPPET_MLCOMMENT_OPEN, this->current())) {
      curr_char = this->lex_mlcomment();
    } else if(find_prefix_in_strv(this->current(), PUPPET_LBRACKETS) >= 0) {
      curr_char = this->lex_lbracket();
    } else if(find_prefix_in_strv(this->current(), PUPPET_RBRACKETS) >= 0) {
      curr_char = this->lex_rbracket();
    } else if(*this->current() == ';') {
      curr_char = this->lex_semicolon();
    } else if(curr_char > 0) {
      this->logger.append_line("LEXER ERROR: Unrecognizable token prefix");
      this->eatchar();
      curr_char = this->peekchar();
      this->status = -1;
    }

    if(curr_char < 0) {
      this->logger.append_line("LEXER ERROR: Bad UTF-8 encoding");
      return -1;
    }
  }

  LexerToken term;
  term.type = TOK_TERM;
  term.lino = this->curr_lino;
  term.chno = this->curr_chno;
  this->token_queue.push_back(term);

  return this->status;
}

int Lexer::lex() {
  int status = this->lex_stage1();
  return status;
}

int Lexer::init(const char *source) {
  this->curr_lino = 1;
  this->curr_chno = 1;
  this->token_queue = vector<LexerToken>();
  this->logger.init();
  this->status = this->data.construct_from_bytes(source);
  if(this->status < 0) {
    this->logger.append_line("LEXER ERROR: Bad UTF-8 Encoding");
  }
  return this->status;
}
