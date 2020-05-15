#include <lisp/vm.h>

enum RISKI_ERROR_CODE vm_load(const char* loc) {
  struct token_list* tl;

  TRACE(lex_file(loc, &tl));

  size_t num_tokens = 0;
  TRACE(lex_num_tokens(&tl, &num_tokens));
  printf("processed %lu tokens\n", num_tokens);

  struct ast* execution_graph = NULL;
  TRACE(ast_build(tl, &execution_graph));

  return RISKI_ERROR_CODE_NONE;
}
