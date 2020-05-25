#ifndef TRACER_
#define TRACER_

#ifndef LOGGER_
#include <logger.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef __FILENAME__
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef PTR_CHECK
#define PTR_CHECK(VAR, ERROR_CODE, ERROR_CODE_STR)                     \
  do {                                                                 \
    if (VAR == NULL) {                                                 \
      logger_error(ERROR_CODE, __func__, __FILENAME__, __LINE__, "%s", \
                   ERROR_CODE_STR[ERROR_CODE]);                        \
      return ERROR_CODE;                                               \
    }                                                                  \
  } while (0)
#endif

#ifndef TRACE
#define TRACE(FUNCTION_CALL)                                             \
  do {                                                                   \
    size_t TRACE_RETURN = FUNCTION_CALL;                                 \
    if (TRACE_RETURN > 0) {                                              \
      logger_error(TRACE_RETURN, __func__, __FILENAME__, __LINE__, "%s", \
                   RISKI_ERROR_TEXT[TRACE_RETURN]);                      \
      return TRACE_RETURN;                                               \
    }                                                                    \
  } while (0)
#endif

#ifndef TRACE_HAULT
#define TRACE_HAULT(FUNCTION_CALL)                                       \
  do {                                                                   \
    size_t TRACE_RETURN = FUNCTION_CALL;                                 \
    if (TRACE_RETURN > 0) {                                              \
      logger_error(TRACE_RETURN, __func__, __FILENAME__, __LINE__, "%s", \
                   RISKI_ERROR_TEXT[TRACE_RETURN]);                      \
      exit(TRACE_RETURN);                                                \
    }                                                                    \
  } while (0)

#endif

#ifndef RANGE_CHECK
#define RANGE_CHECK(VAR, MIN, MAX, ERROR_CODE, ERROR_CODE_STR)         \
  do {                                                                 \
    if (!((int)VAR >= (int)MIN && (int)VAR < (int)MAX)) {              \
      logger_error(ERROR_CODE, __func__, __FILENAME__, __LINE__, "%s", \
                   ERROR_CODE_STR[ERROR_CODE]);                        \
      return ERROR_CODE;                                               \
    }                                                                  \
  } while (0)
#endif

#ifndef COMPARISON_CHECK
#define COMPARISON_CHECK(V1, V2, OP, ERROR_CODE, ERROR_CODE_STR)       \
  do {                                                                 \
    if (!(V1 OP V2)) {                                                 \
      logger_error(ERROR_CODE, __func__, __FILENAME__, __LINE__, "%s", \
                   ERROR_CODE_STR[ERROR_CODE]);                        \
      return ERROR_CODE;                                               \
    }                                                                  \
  } while (0)
#endif

#endif
