#include <logger.h>

__attribute__((__format__(__printf__, 4, 0))) enum RISKI_ERROR_CODE
logger_info(const char *func, const char *filename, int line, const char *fmt,
            ...) {
  time_t rawtime = time(&rawtime);
  struct tm *utc = gmtime(&rawtime);

  flockfile(stdout);

  // print the time
  printf("[%04d-%02d-%02d %02d:%02d:%02d]", (utc->tm_year + 1900),
         (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
         (utc->tm_sec));

  // type
  printf("[\x1b[34mINFO\x1b[0m]");

  // where
  printf("[%s@%s:%d] ", func, filename, line);

  // message
  va_list myargs;
  va_start(myargs, fmt);
  vprintf(fmt, myargs);
  va_end(myargs);

  printf("\n");
  funlockfile(stdout);
  return RISKI_ERROR_CODE_NONE;
}

__attribute__((__format__(__printf__, 4, 0))) enum RISKI_ERROR_CODE
logger_warning(const char *func, const char *filename, int line,
               const char *fmt, ...) {
  time_t rawtime = time(&rawtime);
  struct tm *utc = gmtime(&rawtime);

  flockfile(stdout);

  // print the time
  printf("[%04d-%02d-%02d %02d:%02d:%02d]", (utc->tm_year + 1900),
         (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
         (utc->tm_sec));

  // type
  printf("[\x1b[33mWARNING\x1b[0m]");

  // where
  printf("[%s@%s:%d] ", func, filename, line);

  // message
  va_list myargs;
  va_start(myargs, fmt);
  vprintf(fmt, myargs);
  va_end(myargs);

  printf("\n");
  funlockfile(stdout);

  return RISKI_ERROR_CODE_NONE;
}

__attribute__((__format__(__printf__, 6, 0))) enum RISKI_ERROR_CODE
logger_analysis(const char *security, const char *analysis_name,
                const char *func, const char *filename, int line,
                const char *fmt, ...) {
  (void)analysis_name;
  time_t rawtime = time(&rawtime);
  struct tm *utc = gmtime(&rawtime);

  flockfile(stdout);

  // print the time
  printf("[%04d-%02d-%02d %02d:%02d:%02d]", (utc->tm_year + 1900),
         (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
         (utc->tm_sec));

  // type
  printf("[\x1b[35mANALYSIS\x1b[0m][\x1b[35m%-10s\x1b[0m]", security);

  // where
  printf("[%s@%s:%d]", func, filename, line);

  // message
  va_list myargs;
  va_start(myargs, fmt);
  vprintf(fmt, myargs);
  va_end(myargs);

  printf("\n");
  funlockfile(stdout);
  return RISKI_ERROR_CODE_NONE;
}

__attribute__((__format__(__printf__, 5, 0))) enum RISKI_ERROR_CODE
logger_error(enum RISKI_ERROR_CODE err, const char *func, const char *filename,
             int line, const char *fmt, ...) {
  time_t rawtime = time(&rawtime);
  struct tm *utc = gmtime(&rawtime);

  flockfile(stdout);

  // print the time
  printf("%04d-%02d-%02d/%02d:%02d:%02d/", (utc->tm_year + 1900), (utc->tm_mon),
         (utc->tm_mday), (utc->tm_hour), (utc->tm_min), (utc->tm_sec));

  // type
  printf("ERROR/");

  // where
  printf("%s@%s:%d/", func, filename, line);

  // message
  va_list myargs;
  va_start(myargs, fmt);
  vprintf(fmt, myargs);
  va_end(myargs);

  printf("\n");
  funlockfile(stdout);

  return err;
}
