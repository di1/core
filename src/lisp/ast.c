#include <lisp/ast.h>

#define INDENTATION_LEVEL 2

enum RISKI_ERROR_CODE ast_parse_s_expression(struct token_list* tl, size_t* idx,
                                             struct ast* root);

const char* ast_node_type_text[AST_NODE_TYPE_NUMS] = {
  "PROGRAM",
  "COMMENT",
  "ATOMIC",
  "S_EXPRESSION",
  "LIST"
};

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
 * @param {struct token*} tok The token value, although
 * this may be NULL if there is no token value needed
 * @param {struct ast**} ret Will set *ast to the newly created ast
 */
enum RISKI_ERROR_CODE ast_new(enum AST_NODE_TYPE type, struct token* tok,
                              struct ast** ret) {
  PTR_CHECK(ret, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  struct ast* node = NULL;

  node = (struct ast*)malloc(1 * sizeof(struct ast));
  node->children = NULL;
  node->num_children = 0;
  node->tok = tok;
  node->type = type;
  *ret = node;

  return RISKI_ERROR_CODE_NONE;
}

/**
 * Appends a child to the parent abstract node.
 * @param {struct ast*} parent The parent node to add to
 * @param {struct ast*} child The child node
 */
enum RISKI_ERROR_CODE ast_append_child(struct ast* parent, struct ast* child) {
  PTR_CHECK(parent, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(child, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  parent->num_children += 1;
  parent->children = (struct ast**)realloc(
      parent->children, parent->num_children * sizeof(struct ast*));
  PTR_CHECK(parent->children, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  parent->children[parent->num_children - 1] = child;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE ast_parse_list(struct token_list* tl, size_t* idx,
                                     struct ast* root) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(idx, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(root, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  TRACE(ast_skip_comments(idx, tl));

  // make sue the beginning character is an opar
  enum LEXER_TOKEN tok_value = LEXER_TOKENS_NUM;
  TRACE(lex_token_type(tl, *idx, &tok_value));

  if (tok_value != LEXER_TOKEN_OPAR) {
    printf("error: expected (\n");
    return RISKI_ERROR_CODE_UNKNOWN;
  }

  // increase token ptr
  (*idx) += 1;

  // create child
  struct ast* list_child = NULL;
  struct token* tok = NULL;

  TRACE(lex_get_tok(tl, *idx, &tok));
  TRACE(ast_new(AST_NODE_TYPE_LIST, tok, &list_child));
  TRACE(ast_append_child(root, list_child));

  TRACE(lex_token_type(tl, *idx, &tok_value));

  while (tok_value != LEXER_TOKEN_CPAR) {
    TRACE(ast_parse_s_expression(tl, idx, list_child));
    TRACE(lex_token_type(tl, *idx, &tok_value));
  }

  // check cpar
  if (tok_value != LEXER_TOKEN_CPAR) {
    printf("error: expected )\n");
    return RISKI_ERROR_CODE_NONE;
  }

  // increase ptr
  (*idx) += 1;

  return RISKI_ERROR_CODE_NONE;
}

/**
 * Parses an atomic token
 * @param {struct token_list*} tl The token
 * @param {size_t} idx The index of the token
 * @param {struct ast*} root The root child to add the atomic to
 */
enum RISKI_ERROR_CODE ast_parse_atomic(struct token_list* tl, size_t* idx,
                                       struct ast* root) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(idx, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(root, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  TRACE(ast_skip_comments(idx, tl));

  enum LEXER_TOKEN tok_value = LEXER_TOKENS_NUM;
  TRACE(lex_token_type(tl, *idx, &tok_value));

  if (tok_value != LEXER_TOKEN_ATOMIC) {
    printf("error: expected token got something else\n");
    return RISKI_ERROR_CODE_UNKNOWN;
  }

  struct ast* child = NULL;
  struct token* tok = NULL;

  TRACE(lex_get_tok(tl, *idx, &tok));
  TRACE(ast_new(AST_NODE_TYPE_ATOMIC, tok, &child));

  TRACE(ast_append_child(root, child));

  // increase token ptr
  (*idx) += 1;

  return RISKI_ERROR_CODE_NONE;
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
enum RISKI_ERROR_CODE ast_parse_s_expression(struct token_list* tl, size_t* idx,
                                             struct ast* root) {
  // make sure root isn't null
  PTR_CHECK(root, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // skip comments if we have any we don't need them to build
  // the call tree
  TRACE(ast_skip_comments(idx, tl));

  // get the current token value
  enum LEXER_TOKEN tok_value = LEXER_TOKENS_NUM;
  TRACE(lex_token_type(tl, *idx, &tok_value));

  // branch depending on the token
  switch (tok_value) {
    case LEXER_TOKEN_ATOMIC:
      TRACE(ast_parse_atomic(tl, idx, root));
      return RISKI_ERROR_CODE_NONE;
      break;
    case LEXER_TOKEN_OPAR:
      // TODO
      // this is valid create the list
      TRACE(ast_parse_list(tl, idx, root));
      return RISKI_ERROR_CODE_NONE;
      break;
    case LEXER_TOKEN_CPAR:
      return RISKI_ERROR_CODE_NONE;
      break;
    default:
      printf("compile error\n");
      return RISKI_ERROR_CODE_INVALID_FILE;
      break;
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
  size_t idx = 0;
  while (idx < num_tokens) {
    TRACE(ast_parse_s_expression(tl, &idx, root));
  }

  *execution_graph = root;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE ast_pp(struct ast* root, size_t lvl) {
  PTR_CHECK(root, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  switch (root->type) {
    case AST_NODE_TYPE_PROGRAM:
    case AST_NODE_TYPE_S_EXPRESSION:
    case AST_NODE_TYPE_LIST:
      printf("%*c%s\n", (int)lvl, ' ', ast_node_type_text[root->type]);
      for (size_t i = 0; i < root->num_children; ++i) {
        TRACE(ast_pp(root->children[i], lvl + INDENTATION_LEVEL));
      }
      break;
    case AST_NODE_TYPE_COMMENT:
      break;
    case AST_NODE_TYPE_ATOMIC:
      printf("%*c%s\n", (int)lvl, ' ', ast_node_type_text[root->type]);
      char* val = NULL;
      TRACE(lex_token_value(root->tok, &val));
      printf("%*c%s\n", (int)lvl + INDENTATION_LEVEL, ' ', val);
      break;
    default:
      printf("something is seriously wrong\n");
      break;
  }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE atomic_to_asm(char* val) {
  // check for build in functions first
  // they have a special op code
  if (strcmp(val, "print") == 0) {
    printf("print\n");
  } else if (strcmp(val, "defvar") == 0) {
    printf("assign\n");
  } else if (val[0] == '"') {
    size_t l = 0;
    l = strlen(val) - 2;
    printf("push_str %lu %.*s\n", l, (int)l, &val[1]);
  } else {
  }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE ast_cc(struct ast* root) {
  PTR_CHECK(root, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  switch (root->type) {
    case AST_NODE_TYPE_PROGRAM:
      for (size_t i = 0; i < root->num_children; ++i) {
        TRACE(ast_cc(root->children[i]));
      }
      break;
    case AST_NODE_TYPE_S_EXPRESSION:
    case AST_NODE_TYPE_LIST:
      for (int i = root->num_children-1; i >= 0; --i) {
        TRACE(ast_cc(root->children[i]));
      }
      break;
    case AST_NODE_TYPE_COMMENT:
      break;
    case AST_NODE_TYPE_ATOMIC: {
      char* val = NULL;
      TRACE(lex_token_value(root->tok, &val));
      TRACE(atomic_to_asm(val));
                               }
      break;
    default:
      printf("something is seriously wrong\n");
      break;
  }
  return RISKI_ERROR_CODE_NONE;

}

#undef INDENTATION_LEVEL
