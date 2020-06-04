#ifndef HORIZONTAL_LINE_
#define HORIZONTAL_LINE_

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
#include <tracer.h>

/*
 * Finds horizontal line trends
 * @param {chart*} cht The chart to analyize
 * @param {size_t} end_candle_index The last validated candle
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE find_horizontal_line(struct chart *cht,
                                           size_t end_candle_index);

#endif
