#ifndef SMAKEL_
#define SMAKEL_

#include <error_codes.h>
#include <logger.h>
#include <stdint.h>
#include <string.h>
#include <tracer.h>

/**
 * Enumeration to keep track of lexer types
 */
enum LEXER_TOKEN {
  LEXER_TOKEN_OPAR,
  LEXER_TOKEN_CPAR,
  LEXER_TOKEN_ATOMIC,
  LEXER_TOKEN_COMMENT,
  LEXER_TOKEN_QUOTE,
  LEXER_TOKEN_SEMICOLON,
  LEXER_TOKEN_NEW_LINE,
  LEXER_TOKEN_STRING,
  LEXER_TOKENS_NUM
};

/**
 * Private declaration for the list of tokens
 */
struct token_list;

/**
 * Private declaration for the tokens inside the
 * token list
 */
struct token;

/**
 * Creates tokens from a file
 */
enum RISKI_ERROR_CODE lex_file(const char* file, struct token_list** ret);

/**
 * Gets the number of tokens processed
 */
enum RISKI_ERROR_CODE lex_num_tokens(struct token_list* tl, size_t* ret);

/**
 * Gets the token value of a specified index
 */
enum RISKI_ERROR_CODE lex_token_type(struct token_list* tl,size_t idx, 
    enum LEXER_TOKEN* tok);

#endif
