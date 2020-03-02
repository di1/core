#ifndef CHART_
#define CHART_

#include <analysis/analysis.h>
#include <chart/candle.h>
#include <log/log.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

/**
 *  Private chart struct
 */
struct chart;

/**
 * Creates a new chart given the interval of data
 */
struct chart* chart_new(uint64_t interval, char* name);

/**
 * Frees the chart
 */
void chart_free(struct chart** cht);

/**
 * Updates the chart given the chart, price, and timestamp
 * of the price data
 */
void chart_update(struct chart* cht, int64_t price, uint64_t ts);

/**
 * Gets the name of the chart
 */
char* chart_get_name(struct chart* cht);

/**
 * Converst the chart to a json object
 */
char* chart_json(struct chart* cht);

/**
 * Gets the latest candle update
 */
char* chart_latest_candle(struct chart* cht);

/**
 * enumation for candle patterns
 */
enum SINGLE_CANDLE_PATTERNS {
  SINGLE_CANDLE_PATTERN_NONE,
  SINGLE_CANDLE_PATTERN_WHITE_MARUBOZU,
  SINGLE_CANDLE_PATTERN_BLACK_MARUBOZU,
  SINGLE_CANDLE_PATTERN_WHITE_SPINNING_TOP,
  SINGLE_CANDLE_PATTERN_BLACK_SPINNING_TOP,
  SINGLE_CANDLE_PATTERN_DOJI_DRAGONFLY,
  SINGLE_CANDLE_PATTERN_DOJI_GRAVESTONE,
  SINGLE_CANDLE_PATTERN_DOJI_GENERIC
};

/**
 * enumeration of candle parts
 */
enum DIRECTION {
  DIRECTION_SUPPORT = 0,
  DIRECTION_RESISTANCE = 1,
  DIRECTION_INVALIDATED_SUPPORT = 2,
  DIRECTION_INVALIDATED_RESISTANCE = 3
};

/**
 * Aquires the analysis lock
 */
void chart_analysis_lock(struct chart* cht);

/**
 * Releases the analysis lock
 */
void chart_analysis_unlock(struct chart* cht);

/**
 * Tags a candle with a single candle pattern
 */
void chart_put_single_candle_pattern(struct chart* cht, size_t index,
                                     enum SINGLE_CANDLE_PATTERNS identifier);

/**
 * Markes a line on a chart given two candles, and a boolean "direction"
 * which indicates weather this line should be draw from the lows or the highs
 */
void chart_put_horizontal_line_pattern(struct chart* cht, size_t start,
                                       size_t end, enum DIRECTION direction);

/**
 * Invalidates trends that are currently broken
 */
void chart_invalidate_trends(struct chart* cht);

/**
 * Returns a json that represents the analysis
 */
char* chart_analysis_json(struct chart* cht);

/**
 * Returns a candle, this will only return finalized candles, ie
 * candles that are not currently being modified
 */
struct candle* chart_get_candle(struct chart* cht, size_t index);

/**
 * Runs tests on the chart class
 */
void test_chart();

#endif
