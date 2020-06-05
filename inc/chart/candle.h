#ifndef CANDLE_
#define CANDLE_

#include <error_codes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string_builder.h>
#include <tracer.h>

#define JSON_CANDLE_MAX_LEN 200

/*
 * Private candle struct
 */
struct candle;

/**
 * Creates a new candle given the start time and opening
 * price.
 * @param price A fixed point
 * @param time The timestamp
 * @param cnd Sets *cnd to the newly created candle
 * @return The status
 */
enum RISKI_ERROR_CODE candle_new(int64_t price, int64_t bid, int64_t ask, uint64_t time, struct candle **cnd);

/**
 * Updates the given candle
 * @param c The candle to update
 * @param price The price to update this candle to
 * @param time The timestamp this price happened at
 * @return The status
 */
enum RISKI_ERROR_CODE candle_update(struct candle *c, int64_t price, int64_t bid,
    int64_t ask, uint64_t time);

/**
 * Frees the given candle
 * @param c Will free *c if *c was created with candle_new
 * @return The status
 */
enum RISKI_ERROR_CODE candle_free(struct candle **c);

/**
 * Returns a json representing the candle
 * @param c The candle to get a json representation of
 * @param json Will set *json to the json representation of c.
 * @return The status
 */
enum RISKI_ERROR_CODE candle_json(struct candle *c, char **json);

/**
 * Gets the volume of the candle, 0 if the candle is a fill in candle
 * @param c The candle to get the volume from
 * @param vol Will set *vol to the volume
 * @return The status
 */
enum RISKI_ERROR_CODE candle_volume(struct candle *c, uint64_t *vol);

/**
 * Gets the open price of the candle
 * @param c The candle to get the volume from
 * @param open Will set *open to the open price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_open(struct candle *c, int64_t *open);

/**
 * Gets the high price of the candle
 * @param c The candle to get the volume from
 * @param high Will set *high to the high price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_high(struct candle *c, int64_t *high);

/**
 * Gets the low price of the candle
 * @param c The candle to get the volume from
 * @param low Will set *low to the low price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_low(struct candle *c, int64_t *low);

/**
 * Gets the close price of the candle
 * @param c The candle to get the volume from
 * @param close Will set *close to the close price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_close(struct candle *c, int64_t *close);

/**
 * Gets the open timestamp of the candle
 * @param c The candle to get the volume from
 * @param start Will set *start to the start price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_start(struct candle *c, uint64_t *start);

/**
 * Gets the open timestamp of the candle
 * @param c The candle to get the volume from
 * @param end Will set *end to the end price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_start(struct candle *c, uint64_t *end);

/**
 * Gets the end timestamp of the candle
 * @param c The candle to get the volume from
 * @param end Will set *end to the end price
 * @return The status
 */
enum RISKI_ERROR_CODE candle_end(struct candle *c, uint64_t *end);

#endif
