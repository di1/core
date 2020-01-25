#ifndef CANDLE_
#define CANDLE_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>

#define JSON_CANDLE_MAX_LEN 170

/**
 * Private candle struct
 */
struct candle;

/**
 * Creates a new candle given the start time and opening
 * price, the price is a fixed point number with 4 assumed
 * decimal places
 */
struct candle* candle_new(int64_t price, uint64_t time);

/**
 * Updates the given candle
 */
void candle_update(struct candle* c, int64_t price, uint64_t time);

/**
 * Frees the given candle
 */
void candle_free(struct candle** c);


/**
 * Returns a json representing the candle
 */
char* candle_json(struct candle* c);

/**
 * Runs tests on candles
 */
void test_candle();

#endif
