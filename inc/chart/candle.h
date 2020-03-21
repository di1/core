#ifndef CANDLE_
#define CANDLE_

#include <error_codes.h>
#include <log/log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>

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
 * @param {struct candle**} cnd Sets *cnd to the newly created candle
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_new(int64_t price, uint64_t time,
                                 struct candle** cnd);

/**
 * Updates the given candle
 * @param {struct candle*} c The candle to update
 * @param {int64_t} price The price to update this candle to
 * @param {uint64_t} time The timestamp this price happened at
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_update(struct candle* c, int64_t price,
                                    uint64_t time);

/**
 * Frees the given candle
 * @param {struct candle**} c Will free *c if *c was created with candle_new
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_free(struct candle** c);

/**
 * Returns a json representing the candle
 * @param {struct candle*} c The candle to get a json representation of
 * @param {char**} json Will set *json to the json representation of c.
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_json(struct candle* c, char** json);

/**
 * Gets the volume of the candle, 0 if the candle is a fill in candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {uint64_t*} vol Will set *vol to the volume
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_volume(struct candle* c, uint64_t* vol);

/**
 * Gets the open price of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {int64_t*} open Will set *open to the open price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_open(struct candle* c, int64_t* open);

/**
 * Gets the high price of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {int64_t*} high Will set *high to the high price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_high(struct candle* c, int64_t* high);

/**
 * Gets the low price of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {int64_t*} open Will set *low to the low price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_low(struct candle* c, int64_t* low);

/**
 * Gets the close price of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {int64_t*} open Will set *close to the close price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_close(struct candle* c, int64_t* close);

/**
 * Gets the open timestamp of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {uint64_t*} start Will set *start to the start price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_start(struct candle* c, uint64_t* start);

/**
 * Gets the open timestamp of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {uint64_t*} end Will set *end to the end price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_start(struct candle* c, uint64_t* end);

/**
 * Gets the end timestamp of the candle
 * @param {struct candle*} c The candle to get the volume from
 * @param {uint64_t*} end Will set *end to the end price
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE candle_end(struct candle* c, uint64_t* end);

#endif
