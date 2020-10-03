#include "parser.hpp"
#include "puppet_types.hpp"

DerivationTree *Parser::parse(const LexerToken *q) {
  DerivationTree *root = new DerivationTree;
  root->type = DT_ROOT;
  root->lino = 0;
  root->chno = 0;
  root->next = 0;
  root->children = 0;

  this->queue = q;

  DerivationTree *child;
  DerivationTree **child_ptr = &root->children;
  const LexerToken *curr_queue = this->queue;
  while(this->queue->type != TOK_TERM) {
    child = 0;
    if(curr_queue->type == TOK_LBRACKET) {
      switch(curr_queue->brack_type) {
        case BRACKET_CURLY:
          child = parse_object();
          break;
      }
    }
    if(child) {
      *child_ptr = child;
      child->next = 0;
      child_ptr = &child->next;
    }
    curr_queue = this->queue;
  }

  return root;
}

static PuppetData obj_from_parser(Parser *parser) {
  const LexerToken *local_queue = parser->queue + 1;
  PuppetObject *new_obj = new PuppetObject;
  PuppetData data;

  new_obj->init();
  data.type = TYPE_OBJECT;
  data.obj = new_obj;

  while(local_queue->type != TOK_RBRACKET && 
        local_queue->brack_type != BRACKET_CURLY &&
        local_queue->type != TOK_TERM) {
    if(local_queue->type != TOK_STRING && 
       local_queue->type != TOK_IDENTIFIER) {
       /* Error: Keys must be strings or identifiers  */
       goto bad_obj;

    } else {
      PuppetString lit;
      PuppetData new_data;
      lit.init_literal(local_queue->strliteral);
      local_queue++;
      if(local_queue->type != TOK_OPERATOR || local_queue->op != OP_COLON) {
        /* Error: Must have the colon to separate keys + values */
        goto bad_obj;
      }
      local_queue++;
      switch(local_queue->type) {
        case TOK_LBRACKET:
          switch(local_queue->brack_type) {
            case BRACKET_CURLY:
              break;
            case BRACKET_SQUARE:
              break;
            case BRACKET_PAREN:
              break;
          }
        break;
        case TOK_BIGINT:
          new_data.type = TYPE_BIGINT;
          new_data.i = new PuppetBigInt;
          *new_data.i = local_queue->bigint;
          new_obj->add_pair(lit, new_data);
          local_queue++;
          break;
        default:
          fprintf(stderr, "Not implemented\r\n");
          local_queue++;
      }
    }
    if(local_queue->type != TOK_RBRACKET && local_queue->brack_type != BRACKET_CURLY
       && (local_queue->type != TOK_OPERATOR || local_queue->op != OP_COMMA)) {
      /* Error, must have comma separators or terminate with right curly bracket. */
      goto bad_obj;
    }
    if(local_queue->type == TOK_OPERATOR && local_queue->op == OP_COMMA) {
      local_queue++;
    }
  }
  if(local_queue->type == TOK_TERM) {
    /* Error: Missing right curly bracket*/
  }
  goto regardless;

  bad_obj:
    while(local_queue->type != TOK_RBRACKET &&
          local_queue->brack_type != BRACKET_CURLY &&
          local_queue->type != TOK_TERM) {
      local_queue++;
    }    

  regardless:
    if(local_queue->type != TOK_TERM) {
      local_queue++;
    }
    parser->queue = local_queue;
    return data;
}

DerivationTree *Parser::parse_object() {
  DerivationTree* node = new DerivationTree;

  node->next = 0;
  node->children = 0;
  node->lino = this->queue->lino;
  node->chno = this->queue->chno;
  PuppetData obj = obj_from_parser(this);
  node->obj = obj;

  return node;
}