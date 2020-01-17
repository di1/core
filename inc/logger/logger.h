#ifndef LOGGER_
#define LOGGER_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#define LEVEL_INFO  "[ INFO ]"
#define LEVEL_WARN  "[ WARN ]"
#define LEVEL_FATAL "[ ERRO ]"

void printf_log(const char* lvl, const char* fmt, ...);

#endif
