#include <stdlib.h>

#ifndef PTR_CHECK
#define PTR_CHECK(VAR, ERROR_CODE, ERROR_CODE_STR)                          \
  do {                                                                      \
    if (VAR == NULL) {                                                      \
      printf("error: %s@%s:%d => %s\n", __func__, __FILE__, __LINE__,       \
          ERROR_CODE_STR[ERROR_CODE]);                                      \
      return ERROR_CODE;                                                    \
    }                                                                       \
  } while (0)
#endif

#ifndef TRACE
#define TRACE(FUNCTION_CALL)                                                \
  do {                                                                      \
    if (FUNCTION_CALL != 0) {                                               \
      printf("  |--: %s@%s:%d\n", __func__, __FILE__, __LINE__);            \
      return -1;                                                            \
    }                                                                       \
  while (0) 
#endif
