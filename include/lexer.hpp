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

enum TOK_KEYWORD_TYPE {
  KW_SEND, KW_KEYS, KW_FIND, KW_GET,
  KW_ALL, KW_EXECUTE, KW_ASYNC, KW_CLICK,
  KW_URL, KW_TAG, KW_CSS, KW_ID, KW_XPATH,
  KW_SLEEP
};

enum TOK_OPERATOR_TYPE {
  /* 
   *  Hold off on this. The order 
   *  on this enum matters.
   */
};

extern const char *PUPPET_KEYWORDS[];
extern const char *PUPPET_OPERATORS[];
extern const char *PUPPET_LINE_SEP;
extern const char *PUPPET_SLCOMMENT;
extern const char *PUPPET_MLCOMMENT_OPEN;
extern const char *PUPPET_MLCOMMENT_CLOSE;

struct lexer_token {
  int identifier;
  int lino;
  int chno;
  union {
    int brack_type;
    int keyword;
    const char *identifier;
    mpz_t bigint;
    double flt;
    const char *strliteral;
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
  int lex_stage1();
  unichar_t lex_slcomment();
  unichar_t lex_mlcomment();
  unichar_t lex_number();
  unichar_t lex_identifier();
};



#endif