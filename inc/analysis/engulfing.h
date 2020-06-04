#ifndef ENGULFING_
#define ENGULFING_

#include <analysis/enumations.h>

#ifndef CANDLE_
#include <chart/candle.h>
#endif

#ifndef CHART_
#include <chart/chart.h>
#endif

#include <error_codes.h>
#include <tracer.h>

/*
 * Checks for bullish engulfing candle pattern.
 * @param {size_t} idx The index of the last candle
 * @param {struct chart*} cht The chart to check for bullish engulfing
 * @param {enum SINGLE_CANDLE_PATTERNS*} res The result
 * @param {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE is_bullish_engulfing(size_t idx, struct chart *cht,
                                           enum DOUBLE_CANDLE_PATTERNS *res);

#endif
