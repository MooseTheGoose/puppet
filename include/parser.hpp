#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "puppet_types.hpp"

/*
 *  Not complete, of course, but enough
 *  to at least make something basic. 
 */
enum PUPPET_TREE_TYPE {
  DT_BIGINT, DT_FLOAT, DT_OBJ, DT_LIST,
  DT_IDENTIFIER, DT_OPERATOR, DT_STATEMENT,
  DT_EXPRESSION, DT_KEYWORD, DT_ROOT
};

struct DerivationTree {
  int type;
  int lino;
  int chno;
  DerivationTree *next;
  DerivationTree *children;
  union {
    PuppetData obj;
  };
};

struct Parser {
  const LexerToken *queue;

  DerivationTree *parse(const LexerToken *q);
  DerivationTree *parse_object();
};


#endif