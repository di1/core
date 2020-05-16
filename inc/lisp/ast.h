#ifndef AST_
#define AST_

#include <lisp/lex.h>

/**
 * Holds the abstract syntax tree needed for execution
 */
struct ast;

enum AST_NODE_TYPE {
  AST_NODE_TYPE_COMMENT,
  AST_NODE_TYPE_ATOMIC,
  AST_NODE_TYPE_S_EXPRESSION
};

/**
 * Given a list of tokens builds the abstract syntax tree.
 * Pull set (*ast) to the abstract syntax for vm code generation
 * @param {struct token_list*} tl The token list
 * @param {struct ast**} ret The return value
 */
enum RISKI_ERROR_CODE ast_build(struct token_list* tl, struct ast** ret);

#endif
