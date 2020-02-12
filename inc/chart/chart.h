#ifndef CHART_
#define CHART_

#include <stdlib.h>
#include <stdint.h>
#include <log/log.h>
#include <pthread.h>
#include <chart/candle.h>
#include <analysis/analysis.h>

/**
 *  Private chart struct
 */
struct chart;

/**
 * Creates a new chart given the interval of data
 */
struct chart* chart_new(uint64_t interval);

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
  SINGLE_CANDLE_PATTERN_BLACK_MARUBOZU
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
