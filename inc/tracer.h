#include <stdlib.h>

#ifndef __FILENAME__
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef PTR_CHECK
#define PTR_CHECK(VAR, ERROR_CODE, ERROR_CODE_STR)                        \
  do {                                                                    \
    if (VAR == NULL) {                                                    \
      printf("->%s@%s:%d => %s\n", __func__, __FILENAME__, __LINE__, \
             ERROR_CODE_STR[ERROR_CODE]);                                 \
      return ERROR_CODE;                                                  \
    }                                                                     \
  } while (0)
#endif

#ifndef TRACE
#define TRACE(FUNCTION_CALL)                                         \
  do {                                                               \
    size_t TRACE_RETURN = FUNCTION_CALL;                             \
    if (TRACE_RETURN > 0) {                                          \
      printf("->%s@%s:%d\n", __func__, __FILENAME__, __LINE__); \
      return TRACE_RETURN;                                           \
    }                                                                \
  } while (0)
#endif

#ifndef TRACE_HAULT
#define TRACE_HAULT(FUNCTION_CALL)\
  do {                                                               \
    size_t TRACE_RETURN = FUNCTION_CALL;                             \
    if (TRACE_RETURN > 0) {                                          \
      printf("->%s@%s:%d\n", __func__, __FILENAME__, __LINE__); \
      exit(TRACE_RETURN);                                           \
    }                                                                \
  } while (0)

#endif

#ifndef RANGE_CHECK
#define RANGE_CHECK(VAR, MIN, MAX, ERROR_CODE, ERROR_CODE_STR)            \
  do {                                                                    \
    if (!((int)VAR >= (int)MIN && (int)VAR < (int)MAX)) {                 \
      printf("->%s@%s:%d => %s\n", __func__, __FILENAME__, __LINE__, \
             ERROR_CODE_STR[ERROR_CODE]);                                 \
      return ERROR_CODE;                                                  \
    }                                                                     \
  } while (0)
#endif
