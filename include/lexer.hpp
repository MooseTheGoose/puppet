#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <vector>
#include <string>
#include "puppet_types.hpp"

using std::string;
using std::vector;

enum PUPPET_TOKEN_TYPE {
  TOK_OBJ, TOK_BIGINT, TOK_FLOAT, 
  TOK_STRING, TOK_IDENTIFIER, 
  TOK_KEYWORD, TOK_LBRACKET, TOK_RBRACKET,
  TOK_NL, TOK_SEMICOLON
};

enum TOK_BRACKET_TYPE {
  BRACKET_CURLY, BRACKET_PAREN,
  BRACKET_SQUARE
};

struct lexer_token {
  int identifier;
  int lino;
  int chno;
  union {
    int brack_type;
  };
};



struct lexer {
  int curr_lino;
  int curr_chno;
  const char *data;
  size_t index;
  size_t len;
  vector<lexer_token> token_queue;

  unichar_t peekchar();
  unichar_t eatchar();

  int lex(const string &source);
};



#endif