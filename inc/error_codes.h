#ifndef ERROR_CODES_
#define ERROR_CODES_

#include <stdlib.h>

/*
 * The RISKI error codes
 */
enum RISKI_ERROR_CODE {
  NO_ERROR = 0,
  NULL_PTR = 1,
  MALLOC_ERROR = 2
};


/*
 * The RISKI error texts
 */
extern const char* RISKI_ERROR_TEXT[3];

#endif
