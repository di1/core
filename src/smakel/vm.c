#include <smakel/vm.h>

enum RISKI_ERROR_CODE vm_load(const char* loc) {
  struct token_list* tl;

  TRACE(lex_file(loc, &tl));

  return RISKI_ERROR_CODE_NONE;
}
