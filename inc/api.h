#ifndef API_
#define API_

#ifndef CHART_
#include <chart/chart.h>
#endif

#include <chart/candle.h>
#include <error_codes.h>
#include <tracer.h>

struct vtable
{
  char *(*get_name) (void);
  char *(*get_author) (void);
  enum RISKI_ERROR_CODE (*run) (struct chart *cht, size_t idx);
};

char *get_author ();
char *get_name ();
enum RISKI_ERROR_CODE run (struct chart *cht, size_t idx);

#endif
