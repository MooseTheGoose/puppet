#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include "puppet_types.hpp"

using std::string;
using std::vector;

enum PUPPET_TOKEN_TYPE {
  TOK_OBJ, TOK_BIGINT, TOK_FLOAT, 
  TOK_STRING, TOK_IDENTIFIER, TOK_OPERATOR,
  TOK_KEYWORD, TOK_LBRACKET, TOK_RBRACKET,
  TOK_NL, TOK_SEMICOLON, TOK_TERM
};

enum TOK_BRACKET_TYPE {
  BRACKET_CURLY, BRACKET_PAREN,
  BRACKET_SQUARE
};

enum TOK_KEYWORD_TYPE {
  KW_SEND, KW_KEYS, KW_FIND, KW_GET,
  KW_ALL, KW_EXECUTE, KW_ASYNC, KW_CLICK,
  KW_URL, KW_TAG, KW_CSS, KW_ID, KW_XPATH,
  KW_SLEEP, KW_TO
};

enum TOK_OPERATOR_TYPE {
  /* Order on enum matters. */
};

extern const char *PUPPET_KEYWORDS[];
extern const char *PUPPET_OPERATORS[];
extern const char *const PUPPET_LINE_SEP;
extern const char *const PUPPET_SLCOMMENT;
extern const char *const PUPPET_MLCOMMENT_OPEN;
extern const char *const PUPPET_MLCOMMENT_CLOSE;

struct lexer_token {
  int type;
  int lino;
  int chno;
  union {
    int brack_type;
    int keyword;
    const char *identifier;
    PuppetBigInt bigint;
    PuppetFloat flt;
    const char *strliteral;
    int op;
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

  void init(const char *source);
  int lex();
  int lex_stage1();
  unichar_t lex_slcomment();
  unichar_t lex_mlcomment();
  unichar_t lex_number();
  unichar_t lex_operator();
  unichar_t lex_identifier();
  unichar_t lex_string();
  unichar_t lex_escape(unichar_t &escape);
};



#endif