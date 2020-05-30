#include <logger.h>

bool logger_quite = false;

enum RISKI_ERROR_CODE
logger_info (const char *func, const char *filename, int line, const char *fmt,
             ...)
{
  if (!logger_quite)
    {
      time_t rawtime = time (&rawtime);
      struct tm *utc = gmtime (&rawtime);

      flockfile (stdout);

      // print the time
      printf ("%04d-%02d-%02d/%02d:%02d:%02d/", (utc->tm_year + 1900),
              (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
              (utc->tm_sec));

      // type
      printf ("INFO/");

      // where
      printf ("%s@%s:%d/", func, filename, line);

      // message
      va_list myargs;
      va_start (myargs, fmt);
      vprintf (fmt, myargs);
      va_end (myargs);

      printf ("\n");
      funlockfile (stdout);
    }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
logger_warning (const char *func, const char *filename, int line,
                const char *fmt, ...)
{
  time_t rawtime = time (&rawtime);
  struct tm *utc = gmtime (&rawtime);

  flockfile (stdout);

  // print the time
  printf ("%04d-%02d-%02d/%02d:%02d:%02d/", (utc->tm_year + 1900),
          (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
          (utc->tm_sec));

  // type
  printf ("WARNING/");

  // where
  printf ("%s@%s:%d/", func, filename, line);

  // message
  va_list myargs;
  va_start (myargs, fmt);
  vprintf (fmt, myargs);
  va_end (myargs);

  printf ("\n");
  funlockfile (stdout);

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
logger_analysis (const char *security, const char *analysis_name,
                 const char *func, const char *filename, int line,
                 const char *fmt, ...)
{
  if (!logger_quite)
    {
      time_t rawtime = time (&rawtime);
      struct tm *utc = gmtime (&rawtime);

      flockfile (stdout);

      // print the time
      printf ("%04d-%02d-%02d/%02d:%02d:%02d/", (utc->tm_year + 1900),
              (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
              (utc->tm_sec));

      // type
      printf ("ANALYSIS/%s/%s/", analysis_name, security);

      // where
      printf ("%s@%s:%d/", func, filename, line);

      // message
      va_list myargs;
      va_start (myargs, fmt);
      vprintf (fmt, myargs);
      va_end (myargs);

      printf ("\n");
      funlockfile (stdout);
    }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
logger_error (enum RISKI_ERROR_CODE err, const char *func,
              const char *filename, int line, const char *fmt, ...)
{
  time_t rawtime = time (&rawtime);
  struct tm *utc = gmtime (&rawtime);

  flockfile (stdout);

  // print the time
  printf ("%04d-%02d-%02d/%02d:%02d:%02d/", (utc->tm_year + 1900),
          (utc->tm_mon), (utc->tm_mday), (utc->tm_hour), (utc->tm_min),
          (utc->tm_sec));

  // type
  printf ("ERROR/");

  // where
  printf ("%s@%s:%d/", func, filename, line);

  // message
  va_list myargs;
  va_start (myargs, fmt);
  vprintf (fmt, myargs);
  va_end (myargs);

  printf ("\n");
  funlockfile (stdout);

  return err;
}
