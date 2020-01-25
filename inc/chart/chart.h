#ifndef CHART_
#define CHART_

#include <stdlib.h>
#include <stdint.h>

#include "candle.h"

/**
 *  Private chart struct
 */
struct chart;

/**
 * Creates a new chart given the interval of data
 */
struct chart* chart_new(uint64_t interval);

#endif
