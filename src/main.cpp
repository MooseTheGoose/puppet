#include "puppet_types.hpp"
#include "webdrivers.hpp"
#include "lexer.hpp"
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  lexer lxr = {};
  lxr.init("THIS IS THE WAY \"THE WORLD ENDS\"");

  if(!lxr.lex()) {
    printf("Success!\r\n");
    for(size_t i = 0; i < lxr.token_queue.size(); i++) {
      lexer_token t = lxr.token_queue[i];
      printf("TYPE: %d\r\n", t.type);
      printf("LINE #: %d\r\n", t.lino);
      printf("CHAR #: %d\r\n", t.chno);

      switch(t.type) {
        case TOK_KEYWORD:
          printf("KEYWORD: %s\r\n", PUPPET_KEYWORDS[t.keyword]);
          break;
        case TOK_IDENTIFIER:
          printf("IDENTIFIER: %s\r\n", t.identifier);
          break;
		case TOK_STRING:
		  printf("STRING: \"%s\"\r\n", t.strliteral);
		  break;
      }
    }
  } else {
    printf("Failure!\r\n");
  }

  return 0;
}
