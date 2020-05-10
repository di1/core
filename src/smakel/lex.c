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

/**
 * Holds the list of all tokens processed
 * @param {size_t} the number of tokens in the list
 * @param {struct token*} A list of struct token
 */
struct token_list {
  size_t num_tokens;
  struct token* toks;
};

enum RISKI_ERROR_CODE lex_classify_token(struct token_list* tl, char buf[256]) {
  (void) tl;
  (void) buf;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE lex_file(const char* file, struct token_list** ret) {
  PTR_CHECK(ret, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  FILE* fp = NULL;
  fp = fopen(file, "r");

  if (!fp) {
    return RISKI_ERROR_CODE_INVALID_FILE;
  }

  // create a token list
  struct token_list* tl = NULL;
  tl = (struct token_list*) malloc(sizeof(struct token_list) * 1);
  PTR_CHECK(tl, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  tl->num_tokens = 0;
  tl->toks = NULL;

  // buffer for the last read character
  char chr_ptr = '\x0';

  char token_buffer[256] = {'\x0'}; // max buffer length is 256

  size_t cur_token_buffer_ptr = 0;
  size_t global_line = 1;
  size_t global_column = 1;

  while ( (chr_ptr = fgetc(fp)), chr_ptr != EOF ) {
    switch (chr_ptr) {
      // switch on any delimiter
      case ' ':
      case '\n':
        printf("%s\n", token_buffer);
        memset(token_buffer, '\x0', 256);
        cur_token_buffer_ptr = 0;
        break;
      default:
        token_buffer[cur_token_buffer_ptr] = chr_ptr;
        cur_token_buffer_ptr += 1;

        if (cur_token_buffer_ptr >= sizeof(token_buffer)) {
          // TODO clean up would probably be best practice here
          // TODO even if we just exit the program after this.
          printf("%lu:%lu %s\n", global_line, global_column, token_buffer);
          return RISKI_ERROR_CODE_LEX_TOKEN_TO_BIG;
        }
        break;
    }
  }

  *ret = tl;
  fclose(fp);

  return RISKI_ERROR_CODE_NONE;
}
