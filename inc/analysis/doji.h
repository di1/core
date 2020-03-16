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

#include <tracer.h>

enum DOJI_ERROR_CODE { DOJI_NO_ERROR = 0 };

/*
 * Attempts to find a dragonfly doji pattern on the last confirmed candle.
 * @param {struct candle*} cnd The candle to check
 * @param {struct chart*} cht The chart this candle belongs to
 * @param {size_t} end_candle The last candle
 * @return {enum DOJI_ERROR_CODE} The status
 */
enum DOJI_ERROR_CODE perform_doji_dragonfly(struct chart* cht,
                                            size_t end_candle);

/*
 * Attempts to find a gravestone doji pattern on the last confirmed candle.
 * @param {struct candle*} cnd The candle to check
 * @param {struct chart*} cht The chart this candle belongs to
 * @return {enum DOJI_ERROR_CODE} The status
 */
enum DOJI_ERROR_CODE perform_doji_gravestone(struct chart* cht);

/*
 * Attempts to find a gravestone doji pattern on the last confirmed candle.
 * @param {struct candle*} cnd The candle to check
 * @param {struct chart*} cht The chart this candle belongs to
 * @return {enum DOJI_ERROR_CODE} The status
 */
enum DOJI_ERROR_CODE perform_doji_generic(struct chart* cht);

#endif
