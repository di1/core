#include <lisp/vm.h>

enum RISKI_ERROR_CODE vm_load(const char* loc) {
  // lex the file
  struct token_list* tl;
  TRACE(lex_file(loc, &tl));

  // generate the abstract syntax tree
  struct ast* execution_graph = NULL;
  TRACE(ast_build(tl, &execution_graph));

  // for debugging print out the ast
  TRACE(ast_pp(execution_graph, 2)); 

  // generate bytecode
  TRACE(ast_cc(execution_graph));

  return RISKI_ERROR_CODE_NONE;
}
