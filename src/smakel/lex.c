#include "error_codes.h"
#include "tracer.h"
#include <smakel/lex.h>

/**
 * Stores information about a token that was parsed.
 * We store more to give useful compile errors.
 * @param {size_t} line The line this token was found on
 * @param {size_t} column The column this token was found on
 * @param {size_t} token_length The length of the token
 * @param {enum LEXER_TOKEN} type The type of token this is
 * @param {char*} value The value of the token
 */
struct token {
  size_t line;
  size_t column;
  size_t token_length;

  enum LEXER_TOKEN type;
  char* value;
};

struct token_list {
  size_t num_tokens;
  struct token* toks;
}

enum RISKI_ERROR_CODE lex_file(const char* file, struct token_list** ret) {
  PTR_CHECK(ret, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  FILE* fp = NULL;
  fp = fopen(file, "r");

  if (!fp) {
    return RISKI_ERROR_CODE_INVALID_FILE;
  }

  // max token length buff is 256 characters
  size_t line = 0;
  size_t column = 0;

  // buffer for the last read character
  char chrPtr = '\x0';

  // create a token list
  struct token_list* tl = NULL;
  tl = (struct token_list*) malloc(sizeof(struct token_list) * 1);
  PTR_CHECK(tl, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  tl->num_tokens = 0;
  tl->toks = NULL;

  while ( (chrPtr = fgetc(fp)), chrPtr != EOF ) {
    switch (chrPtr) {
      default:
        break;
    }
  }

  *ret = tl;
  fclose(fp);

  return RISKI_ERROR_CODE_NONE;
}
