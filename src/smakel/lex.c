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
  struct token** toks;
};


enum RISKI_ERROR_CODE lex_token_new(size_t line, size_t column,
    size_t token_length, enum LEXER_TOKEN type, char* value,
    struct token** ret) {

  struct token* tok = NULL;
  tok = (struct token*) malloc(sizeof(struct token) * 1);
  PTR_CHECK(tok, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  tok->line = line;
  tok->column = column;
  tok->token_length = token_length;
  tok->type = type;

  // duplicate the value passed in by value
  tok->value = strdup(value);

  if (token_length == 1) {
    printf("token; value=0x%x line=%lu col=%lu type=%d\n",
      value[0], line, column, type);
  } else {
    printf("token; value=%s line=%lu col=%lu type=%d\n",
      value, line, column, type);
  }

  *ret = tok;

  return RISKI_ERROR_CODE_NONE;
}

/**
 * Given a token list and a character buffer associated with a token,
 * will classify the token and append it to the list
 */
enum RISKI_ERROR_CODE lex_classify_and_token(struct token_list* tl,
    char buf[256], size_t line, size_t column) {

  PTR_CHECK(tl, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  struct token* tok = NULL;

  if (strlen(buf) == 1) {
    // only 1 character is either a delimiter or just a single character
    switch (buf[0]) {
      case ';':
        TRACE(lex_token_new(line, column, 1, LEXER_TOKEN_SEMICOLON, buf,
              &tok));
        break;
      case ' ':
        TRACE(lex_token_new(line, column, 1, LEXER_TOKEN_SPACE, buf,
              &tok));
        break;
      case '\n':
        TRACE(lex_token_new(line, column, 1, LEXER_TOKEN_NEW_LINE, buf,
              &tok));
        break;
      default:
        // just a single character string meh
        TRACE(lex_token_new(line, column, 1, LEXER_TOKEN_STRING, buf,
              &tok));
        break;
    }
  } else {
    if (strcmp(buf, "vr") == 0) {
      // variable keyword
      TRACE(lex_token_new(line, column, 2, LEXER_TOKEN_VR, buf, &tok));
    } else if (strcmp(buf, "fn") == 0) {
      // function keyword
      TRACE(lex_token_new(line, column, 2, LEXER_TOKEN_FN, buf, &tok));
    } else if (strcmp(buf, "::") == 0) {
      // definition declaration
      TRACE(lex_token_new(line, column, 2, LEXER_TOKEN_DOUBLE_COLON, buf,
            &tok));
    } else {
      // generic string
      TRACE(lex_token_new(line, column, strlen(buf), LEXER_TOKEN_STRING, buf,
            &tok));
    }
  }

  if (!tok) {
    // this probably should never happen but its good
    // to put in
    printf("invalid token: %s\n", buf);
    exit(1);
  } else {
    // append the token to the token list
    tl->num_tokens += 1;
    tl->toks = realloc(tl->toks, sizeof(struct token*) * tl->num_tokens);
    tl->toks[tl->num_tokens - 1] = tok;
  }
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
      case ';':
        // classify the text leading up to the delimiter
        if (token_buffer[0] != '\x0') {
          TRACE(lex_classify_and_token(tl, token_buffer, global_line,
                global_column));
          memset(token_buffer, '\x0', 256);
          cur_token_buffer_ptr = 0;
        }
        // classify the delimiter
        memset(token_buffer, '\x0', 256);
        token_buffer[0] = chr_ptr;
        TRACE(lex_classify_and_token(tl, token_buffer, global_line,
              global_column));
        token_buffer[0] = '\x0';
        cur_token_buffer_ptr = 0;

        if (chr_ptr == '\n') {
          global_line += 1;
          global_column = 1;
        } else {
          global_column += 1;
        }

        break;
      default:
        token_buffer[cur_token_buffer_ptr] = chr_ptr;
        cur_token_buffer_ptr += 1;
        global_column += 1;

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
