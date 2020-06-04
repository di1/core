#ifndef TREND_LINE_
#define TREND_LINE_

#ifndef CANDLE_
#include <chart/candle.h>
#else
struct candle;
#endif

#ifndef CHART_
#include <chart/chart.h>
#else
struct chart;
#endif

#include <error_codes.h>
#include <logger.h>

#ifndef INTEGRAL_
#include <math/integral.h>
#endif

#include <math/linear_equation.h>
#include <tracer.h>
/*
 * Find trend lines that aren't horizontal lines
 * @param {struct char*} num_candles The chart
 * @param {size_t} num_candles The last candle that isn't going to change
 */
enum RISKI_ERROR_CODE find_trend_line(struct chart *cht, size_t num_candles,
                                      enum DIRECTION type);

#endif
