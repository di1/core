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
  LEXER_TOKEN_COMMENT
};

/**
 * Private declaration for the token struct
 */
struct token;


#endif
