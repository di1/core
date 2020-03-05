#ifndef TREND_LINE_
#define TREND_LINE_

#include <chart/candle.h>
#include <chart/chart.h>
#include <math/linear_equation.h>

/*
 * Find trend lines that aren't horizontal lines
 * @param {struct char*} num_candles The chart
 * @param {size_t} num_candles The last candle that isn't going to change
 */
void find_trend_line(struct chart* cht, size_t num_candles);

#endif
