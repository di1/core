#ifndef CHART_
#define CHART_

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "candle.h"

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
 * Runs tests on the chart class
 */
void test_chart();

#endif
