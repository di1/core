#ifndef ANALYSIS_
#define ANALYSIS_

#include <chart/candle.h>

// if guard circular dependency
#ifndef CHART_
#include <chart/chart.h>
#else
struct chart;
#endif

#include <analysis/doji.h>
#include <analysis/horizontal_line.h>
#include <analysis/marubozu.h>
#include <analysis/spinning_top.h>
#include <analysis/trend_line.h>


#include <log/log.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Adds a security to be analyzed.
 */
void analysis_add(struct chart* sec);

/**
 * Initalizes threads to perform analysis on
 */
void analysis_init();

/**
 * Pushes information to a thread to analyize it
 */
void analysis_push(struct chart* sec, size_t start, size_t end);

void analysis_cleanup();

#endif
