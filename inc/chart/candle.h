#ifndef CANDLE_
#define CANDLE_

#include <log/log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define JSON_CANDLE_MAX_LEN 200

/*
 * Private candle struct
 */
struct candle;

/*
 * Creates a new candle given the start time and opening
 * price.
 * @param {int64_t} price A fixed point
 * @param {uint64_t} time The timestamp
 */
enum RISKI_ERROR_CODE candle_new(int64_t price, uint64_t time, struct candle** cnd);

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
 * Gets the volume of the candle, 0 if the candle is a fill in candle
 */
uint64_t candle_volume(struct candle* c);

/**
 * Gets the open price of the candle
 */
int64_t candle_open(struct candle* c);

/**
 * Gets the high price of the candle
 */
int64_t candle_high(struct candle* c);

/**
 * Gets the low price of the candle
 */
int64_t candle_low(struct candle* c);

/**
 * Gets the close price of the candle
 */
int64_t candle_close(struct candle* c);

/**
 * Gets the open timestamp of the candle
 */
uint64_t candle_start(struct candle* c);

/**
 * Gets the end timestamp of the candle
 */
uint64_t candle_end(struct candle* c);

/**
 * Runs tests on candles
 */
void test_candle();

#endif
