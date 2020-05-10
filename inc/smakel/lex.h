#ifndef SMAKEL_
#define SMAKEL_

#include <stdint.h>
#include <tracer.h>
#include <error_codes.h>
#include <logger.h>
#include <string.h>

enum LEXER_TOKEN {
  LEXER_TOKEN_VR,
  LEXER_TOKEN_FN,
  LEXER_TOKEN_DOUBLE_COLON,
  LEXER_TOKEN_STRING,
  LEXER_TOKEN_SEMICOLON,
  LEXER_TOKEN_SPACE,
  LEXER_TOKEN_NEW_LINE
};

/**
 * Private declaration for the list of tokens
 */
struct token_list;

enum RISKI_ERROR_CODE lex_file(const char* file, struct token_list** ret);

#endif
