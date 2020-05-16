#include <lisp/lex.h>

#define ATOMIC_LENGTH 256

const char* lexer_token_str_val[LEXER_TOKENS_NUM] = {
  "LEXER_TOKEN_OPAR",
  "LEXER_TOKEN_CPAR",
  "LEXER_TOKEN_ATOMIC",
  "LEXER_TOKEN_COMMENT",
  "LEXER_TOKEN_QUOTE",
  "LEXER_TOKEN_SEMICOLON",
  "LEXER_TOKEN_NEW_LINE",
  "LEXER_TOKEN_STRING"
};

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
                                    size_t token_length, enum LEXER_TOKEN type,
                                    char* value, struct token_list* tl) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  struct token* tok = NULL;
  tok = (struct token*)malloc(sizeof(struct token) * 1);
  PTR_CHECK(tok, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  tok->line = line;
  tok->column = column;
  tok->token_length = token_length;
  tok->type = type;

  // duplicate the value passed in by value
  tok->value = strdup(value);

  // add token to the token list
  tl->num_tokens += 1;
  tl->toks =
      (struct token**)realloc(tl->toks, sizeof(struct token*) * tl->num_tokens);
  tl->toks[tl->num_tokens - 1] = tok;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE lex_token_delete(size_t idx, struct token_list* tl) {
  // free the token
  free(tl->toks[idx]->value);
  tl->toks[idx]->value = NULL;
  free(tl->toks[idx]);
  tl->toks[idx] = NULL;

  // shift
  for (size_t shift_idx = idx+1; shift_idx < tl->num_tokens; ++shift_idx) {
    tl->toks[shift_idx - 1] = tl->toks[shift_idx];
  }
  tl->num_tokens -= 1;

  // free up memory
  tl->toks = realloc(tl->toks, tl->num_tokens * sizeof(struct token));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE lex_token_list_print(struct token_list* tl) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // loop through each token and print out information about them
  for (size_t i = 0; i < tl->num_tokens; ++i) {
    struct token* tok = tl->toks[i];
    PTR_CHECK(tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
    printf("TOKEN TYPE=%-40s LINE=%-3lu COLUMN=%-3lu VALUE=%s\n",
        lexer_token_str_val[tok->type], tok->line, tok->column,
        (tok->type == LEXER_TOKEN_NEW_LINE ? "" : tok->value));
  }

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE lex_flush_buffer(char token_buffer[ATOMIC_LENGTH],
                                       struct token_list* tl,
                                       size_t* cur_token_buffer_ptr,
                                       size_t global_line,
                                       size_t global_column) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  if (token_buffer[0] != '\x0') {
    // flush the buffer
    TRACE(lex_token_new(global_line, global_column, strlen(token_buffer),
                        LEXER_TOKEN_ATOMIC, token_buffer, tl));
    memset(token_buffer, '\x0', ATOMIC_LENGTH);
    *cur_token_buffer_ptr = 0;
  }

  return RISKI_ERROR_CODE_NONE;
}

/**
 * Condense multiple tokens into 1 combining their values
 * and changing the token type.
 * @param {struct token_list*} tl The token list to work with
 * @param {enum LEXER_TOKEN} beg_tok The token that defines the start
 * of the condensation.
 * @param {enum LEXER_TOKEN} end_tok The token that defines the end
 * of the condensation.
 * @param {enum LEXER_TOKEN} res_value The token type the condensed token
 * should have
 * @param {bool} reduce If true will not include the end_tok in the
 * condensation token value.
 */
enum RISKI_ERROR_CODE lex_condense(struct token_list* tl,
    enum LEXER_TOKEN beg_tok, enum LEXER_TOKEN end_tok,
    enum LEXER_TOKEN res_value, bool reduce) {

  for (size_t i = 0; i < tl->num_tokens; ++i) {
    if (tl->toks[i]->type == beg_tok) {
      // loop until a new line character is found
      size_t j = i + 1;
      size_t new_token_length = 1;
      for (; j < tl->num_tokens; ++j) {
        if (tl->toks[j]->type == end_tok) {
          break;
        } else {
          new_token_length += tl->toks[j]->token_length;
        }
      }
      // must increase new_token_length by j to account for the spaces
      // this could allocate extra space
      char* comment_length = (char*)
        malloc((new_token_length + j + 1) * sizeof(char));
      memset(comment_length, '\x0', new_token_length + j + 1);
      // reduce j by one to not include the new line character
      if (reduce || j == tl->num_tokens) {
        j = j - 1;
      }
      // loop backwards and append the string backwards to build the entire
      // comment
      size_t offset = 0;
      for (size_t l = i; l <= j; ++l) {
        strcpy(&comment_length[offset], tl->toks[l]->value);
        offset += tl->toks[l]->token_length;
        if (!(tl->toks[l]->type == LEXER_TOKEN_QUOTE ||
            tl->toks[l+1]->type == LEXER_TOKEN_QUOTE)) {
          comment_length[offset] = ' ';
          offset += 1;
        }
      }
      // change the semi colon token value and type to comment
      free(tl->toks[i]->value);
      tl->toks[i]->value = comment_length;
      tl->toks[i]->token_length = strlen(tl->toks[i]->value);
      tl->toks[i]->type = res_value;

      // delete the tokens that have been condensed
      for (size_t num_to_delete = 0; num_to_delete < (reduce ? j-i+1 : j-i);
          ++num_to_delete) {
        TRACE(lex_token_delete(i+1, tl));
      }
    }
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
  tl = (struct token_list*)malloc(sizeof(struct token_list) * 1);
  PTR_CHECK(tl, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  tl->num_tokens = 0;
  tl->toks = NULL;

  // buffer for the last read character
  char chr_ptr = '\x0';

  char token_buffer[ATOMIC_LENGTH] = {'\x0'};  // max buffer length is 256

  size_t cur_token_buffer_ptr = 0;
  size_t global_line = 1;
  size_t global_column = 0;

  // first pass
  while ((chr_ptr = fgetc(fp)), chr_ptr != EOF) {
    global_column += 1;

    switch (chr_ptr) {
      // switch on any delimiter
      case '(':
        TRACE(lex_flush_buffer(token_buffer, tl, &cur_token_buffer_ptr,
                               global_line, global_column));
        TRACE(lex_token_new(global_line, global_column, 1, LEXER_TOKEN_OPAR,
                            "(", tl));
        break;
      case ')':
        TRACE(lex_flush_buffer(token_buffer, tl, &cur_token_buffer_ptr,
                               global_line, global_column));
        TRACE(lex_token_new(global_line, global_column, 1, LEXER_TOKEN_CPAR,
                            ")", tl));
        break;
      case ' ':
        TRACE(lex_flush_buffer(token_buffer, tl, &cur_token_buffer_ptr,
                               global_line, global_column));
        break;
      case '"':
        TRACE(lex_flush_buffer(token_buffer, tl, &cur_token_buffer_ptr,
                               global_line, global_column));
        TRACE(lex_token_new(global_line, global_column, 1, LEXER_TOKEN_QUOTE,
                            "\"", tl));
        break;
      case ';':
        TRACE(lex_flush_buffer(token_buffer, tl, &cur_token_buffer_ptr,
                               global_line, global_column));
        TRACE(lex_token_new(global_line, global_column, 1,
                            LEXER_TOKEN_SEMICOLON, ";", tl));
        break;
      case '\n':
        TRACE(lex_flush_buffer(token_buffer, tl, &cur_token_buffer_ptr,
                               global_line, global_column));
        TRACE(lex_token_new(global_line, global_column, 1, LEXER_TOKEN_NEW_LINE,
                            "\n", tl));
        global_line += 1;
        global_column = 0;
        break;
      default:
        // any other character push to token_buffer
        token_buffer[cur_token_buffer_ptr] = chr_ptr;
        cur_token_buffer_ptr += 1;
    }
  }

  // get rid of the new line so we reduce
  TRACE(lex_condense(tl, LEXER_TOKEN_SEMICOLON, LEXER_TOKEN_NEW_LINE,
        LEXER_TOKEN_COMMENT, true));
  TRACE(lex_condense(tl, LEXER_TOKEN_QUOTE, LEXER_TOKEN_QUOTE,
        LEXER_TOKEN_STRING, false));

  // get rid of new lines that were not condensed as these new line characters
  // were likely used for formatting
  for (size_t i = 0; i < tl->num_tokens; ++i) {
    if (tl->toks[i]->type == LEXER_TOKEN_NEW_LINE) {
      TRACE(lex_token_delete(i, tl));
      i = 0;
    }
  }

  TRACE(lex_token_list_print(tl));

  *ret = tl;
  fclose(fp);

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE lex_num_tokens(struct token_list* tl, size_t* ret) {
  PTR_CHECK(tl, RISKI_ERROR_CODE_NONE, RISKI_ERROR_TEXT);
  *ret = tl->num_tokens;
  return RISKI_ERROR_CODE_NONE;
}

#undef ATOMIC_LENGTH
