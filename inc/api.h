#ifndef API_
#define API_


#include <chart/chart.h>
#include <chart/candle.h>
#include <error_codes.h>
#include <tracer.h>

char *get_name();
char *get_author();

enum RISKI_ERROR_CODE run(struct chart* cht, size_t idx);

#endif
