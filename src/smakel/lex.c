#include "error_codes.h"
#include "tracer.h"
#include <smakel/lex.h>

struct token {
  size_t line;
  size_t column;
  char value[256];
};

enum RISKI_ERROR_CODE lex_file(const char* file, struct token** tokens) {
  PTR_CHECK(tokens, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  FILE* fp = NULL;
  fp = fopen(file, "r");

  if (!fp) {
    return RISKI_ERROR_CODE_INVALID_FILE;
  }

  // max token length is 256 characters

  fclose(fp);

  return RISKI_ERROR_CODE_NONE;
}
