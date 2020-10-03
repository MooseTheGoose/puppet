#include "puppet_types.hpp"
#include "webdrivers.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "puppet_os.hpp"
#include <stdlib.h>
#include <locale.h>

int main(int argc, const char *argv[]) {
  Lexer lxr;
  Parser prsr;
  setlocale(LC_ALL, "");

  lxr.init("{ my_first: 0x1234, data: 0x5678}");
  if (!lxr.lex()) {
    for (size_t i = 0; lxr.token_queue[i].type != TOK_TERM; i++) {
      LexerToken t = lxr.token_queue[i];
      printf("TYPE: %d\r\n", t.type);
      printf("LINE #: %d\r\n", t.lino);
      printf("CHAR #: %d\r\n", t.chno);

      switch (t.type) {
        case TOK_KEYWORD:
          printf("KEYWORD: %s\r\n", PUPPET_KEYWORDS[t.keyword]);
          break;
        case TOK_IDENTIFIER:
          printf("IDENTIFIER: %s\r\n", t.identifier);
          break;
        case TOK_STRING:
          printf("STRING: \"%s\"\r\n", t.strliteral);
          break;
        case TOK_LBRACKET:
          printf("LEFT BRACKET: %s\r\n", PUPPET_LBRACKETS[t.brack_type]);
          break;
        case TOK_RBRACKET:
          printf("RIGHT BRACKET: %s\r\n", PUPPET_RBRACKETS[t.brack_type]);
          break;
        case TOK_OPERATOR:
          printf("OPERATOR: %s\r\n", PUPPET_OPERATORS[t.op]);
          break;
        case TOK_BIGINT:
          printf("NUMBER: %s\r\n", t.bigint.to_string().data());
          break;
      }
    }
    DerivationTree *root = prsr.parse(lxr.token_queue.data());

  } else {
    printf("%s\r\n", lxr.logger.output.data());
  }
  return 0;
}
