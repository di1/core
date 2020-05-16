#include <lisp/ast.h>

struct ast {
  enum AST_NODE_TYPE type;
  struct token* tok;
  size_t num_children;
  struct ast** children;
};

enum RISKI_ERROR_CODE ast_build(struct token_list* tl,
                                struct ast** execution_graph) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct ast* root = NULL;

  size_t num_tokens = 0;
  TRACE(lex_num_tokens(tl, &num_tokens));
  printf("generating ast from %lu tokens\n", num_tokens);

  *execution_graph = root;
  return RISKI_ERROR_CODE_NONE;
}
