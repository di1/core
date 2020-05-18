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

/**
 * Creates a new ast from a given type
 * @param {enum AST_NODE_TYPE} type The node type
 * @param {struct token*} tok The token that coorisponds to its value, although
 * this may be NULL if there is no coorisponding token
 * @param {struct ast**} ret Will set *ast to the newly created ast
 */
enum RISKI_ERROR_CODE ast_new(enum AST_NODE_TYPE type, struct token* tok,
    struct ast** ret) {

  PTR_CHECK(ret, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  struct ast* node = NULL;
  
  node = (struct ast*) malloc(1 * sizeof(struct ast));
  node->children = NULL;
  node->num_children = 0;
  node->tok = tok;
  node->type = type;
  *ret = node;

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

  // make sure root isn't null
  PTR_CHECK(root, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

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
      break;
    case LEXER_TOKEN_OPAR:
      // TODO
      // this is valid create the list
      break;
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

  // the root of execution is a program
  TRACE(ast_new(AST_NODE_TYPE_PROGRAM, NULL, &root));

  // Parse the lisp
  TRACE(ast_parse_s_expression(tl, 0, root));

  *execution_graph = root;
  return RISKI_ERROR_CODE_NONE;
}
