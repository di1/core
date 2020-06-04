#ifndef API_
#define API_

#ifndef CHART_
#include <chart/chart.h>
#endif

#include <math/linear_equation.h>

#include <chart/candle.h>
#include <error_codes.h>
#include <tracer.h>

struct vtable {
  const char *(*get_name)(void);
  const char *(*get_author)(void);
  enum RISKI_ERROR_CODE (*run)(struct chart *cht, size_t idx);
};

const char *get_author(void);
const char *get_name(void);

enum RISKI_ERROR_CODE run(struct chart *cht, size_t idx);

extern struct vtable exports;
#endif
