#ifndef ERROR_CODES_
#define ERROR_CODES_

#include <stdlib.h>

/*
 * The RISKI error codes
 */
enum RISKI_ERROR_CODE {
  RISKI_ERROR_CODE_NONE = 0,
  RISKI_ERROR_CODE_NULL_PTR = 1,
  RISKI_ERROR_CODE_MALLOC_ERROR = 2,
  RISKI_ERROR_CODE_JSON_CREATION = 3,
  RISKI_ERROR_INVALID_RANGE = 4
};


/*
 * The RISKI error texts
 */
extern const char* RISKI_ERROR_TEXT[5];

#endif
