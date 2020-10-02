#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include "puppet_types.hpp"
#include "puppet_logger.hpp"

using std::string;
using std::vector;

enum PUPPET_TOKEN_TYPE {
  TOK_BIGINT, TOK_FLOAT, TOK_STRING, TOK_IDENTIFIER, 
  TOK_OPERATOR, TOK_KEYWORD, TOK_LBRACKET, TOK_RBRACKET,
  TOK_NL, TOK_SEMICOLON, TOK_TERM
};

enum TOK_BRACKET_TYPE {
  BRACKET_CURLY, BRACKET_PAREN,
  BRACKET_SQUARE
};

enum TOK_KEYWORD_TYPE {
  KW_SEND, KW_FIND, KW_GET,
  KW_ALL, KW_EXECUTE, KW_ASYNC, KW_CLICK,
  KW_URL, KW_TAG, KW_CSS, KW_ID, KW_XPATH,
  KW_SLEEP, KW_TO
};

enum TOK_OPERATOR_TYPE {
  /* Order on enum matters. */
};

extern const char *PUPPET_KEYWORDS[];
extern const char *PUPPET_OPERATORS[];
extern const char *PUPPET_LBRACKETS[];
extern const char *PUPPET_RBRACKETS[];
extern const char *const PUPPET_LINE_SEP;
extern const char *const PUPPET_SLCOMMENT;
extern const char *const PUPPET_MLCOMMENT_OPEN;
extern const char *const PUPPET_MLCOMMENT_CLOSE;

/*
 * Umm... This is awkward. I capitalize some struct
 * names and lowercase others.
 *
 * TODO: Change lexer_token, lexer, and utf8_str to
 *       be capitalized.
 */

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
  StringLogger logger;
  vector<lexer_token> token_queue;
  utf8_str data;
  int curr_lino;
  int curr_chno;
  int status;

  unichar_t peekchar();
  unichar_t eatchar();
  unichar_t eatstr(const char *str);
  const char *current();

  int init(const char *source);
  int lex();
  int lex_stage1();
  unichar_t lex_slcomment();
  unichar_t lex_mlcomment();
  unichar_t lex_number();
  unichar_t lex_operator();
  unichar_t lex_identifier();
  unichar_t lex_string();
  unichar_t lex_lbracket();
  unichar_t lex_rbracket();
  unichar_t lex_semicolon();
  unichar_t lex_escape(unichar_t *escape);
};



#endif