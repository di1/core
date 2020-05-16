#include <lisp/ast.h>

struct ast {
  enum AST_NODE_TYPE type;
  struct token* tok;
  size_t num_children;
  struct ast** children;
};

enum RISKI_ERROR_CODE ast_skip_comments(size_t* idx, struct token_list* tl) {
  enum LEXER_TOKEN tok_value = LEXER_TOKENS_NUM;
  do {
    TRACE(lex_token_type(tl, *idx, &tok_value));
    if (tok_value != LEXER_TOKEN_COMMENT) {
      break;
    }
    *idx += 1;
  } while (1);
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE ast_parse_list() {

}

enum RISKI_ERROR_CODE ast_parse_atomic() {

}

/**
 * Attempts to parse
 *
 * s_expression : {LEX_TOKEN_ATOMIC} atomic
 *              | {LEX_TOKEN_OPAR} list
 *
 * @param {struct token_list*} tl The token list
 * @param {size_t} idx The current token index we are looking at.
 * @param {struct ast*} root The root node we are currently working
 * with.
 */
enum RISKI_ERROR_CODE ast_parse_s_expression(struct token_list* tl,
    size_t idx, struct ast* root) {
  // skip comments if we have any we don't need them to build
  // the call tree
  TRACE(ast_skip_comments(&idx, tl));

  // get the current token value
  enum LEXER_TOKEN tok_value = LEXER_TOKENS_NUM;
  TRACE(lex_token_type(tl, idx, &tok_value));

  // branch depending on the token
  switch (tok_value) {
    case LEXER_TOKEN_ATOMIC:
      // TODO
      // this is valid create the atomic node
    case LEXER_TOKEN_OPAR:
      // TODO
      // this is valid create the list
    default:
      printf("compile error\n");
      return RISKI_ERROR_CODE_INVALID_FILE;
  } 

  return RISKI_ERROR_CODE_NONE;
}

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
