#ifndef PARSER_HPP
#define PARSER_HPP

/*
 *  Not complete, of course, but enough
 *  to at least make something basic. 
 */
enum PUPPET_TREE_TYPE {
  DT_BIGINT, DT_FLOAT, DT_OBJ, DT_LIST,
  DT_IDENTIFIER, DT_OPERATOR, DT_STATEMENT,
  DT_EXPRESSION, DT_KEYWORD
};

struct DerivationTree {
  int type;
  int lino;
  int chno;
  DerivationTree *next;
  union {
    DerivationTree *children;
  }
}

#endif