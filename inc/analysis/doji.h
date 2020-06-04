#ifndef DOJI_
#define DOJI_

#include <analysis/enumations.h>

// if guard circular dependency
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
 * Attempts to find a dragonfly doji pattern on the last confirmed candle.
 * @param {struct candle*} cnd The candle to analyize
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE perform_doji_dragonfly(struct candle *cnd,
                                             enum SINGLE_CANDLE_PATTERNS *res);

/*
 * Attempts to find a gravestone doji pattern on the last confirmed candle.
 * @param {struct candle*} cht The candle to analyzie
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE perform_doji_gravestone(struct candle *cnd,
                                              enum SINGLE_CANDLE_PATTERNS *res);

/*
 * Attempts to find a gravestone doji pattern on the last confirmed candle.
 * @param {struct chart*} cht The candle to analyzie
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE perform_doji_generic(struct candle *cnd,
                                           enum SINGLE_CANDLE_PATTERNS *res);

#endif
