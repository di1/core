#ifndef SECURITY_
#define SECURITY_

#include <book/book.h>
#include <chart/chart.h>
#include <error_codes.h>
#include <logger.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>

#define SECURITY_INTERVAL_MINUTE_NANOSECONDS 6e+10
#define SECURITY_INTERVAL_5SECOND_NANOSECONDS 5000000000
#define SECURITY_INTERVAL_5MINUTE_NANOSECONDS 300000000000

#ifdef RUN_TESTS
#define SECURITY_HASH_MODULE_VAL 2
#else
#define SECURITY_HASH_MODULE_VAL 12000
#endif

/*
 * Private definition of a security
 */
struct security;

/*
 * Creates a new security given a name.
 * @param {char*} name The name of the security
 * @param {uint64_t} interval The interval between candles in a consistent
 * time format
 * @param {struct security**} sec Sets *sec to the newly created security
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_new(char *name, uint64_t interval,
                                   struct security **sec);

/*
 * Updates the order book associated with the security.
 * Side, true, is for the buy side and false is the sell side
 * The price is a a fixed point number. As long as all the data
 * is the same fixed point number this function will perform correctly.
 * You can not mix a fixed 4 decimal number with a fixed 5 decimal number,
 * if done this will result in undefined behavior. The quantity is the number
 * of shares.
 * @param {struct security*} sec The security to update
 * @param {bool} side True for buy False for sell
 * @param {int64_t} price The price level
 * @param {int64_t} quantity The quantity on the price level
 */
enum RISKI_ERROR_CODE security_book_update(struct security *sec, bool side,
                                           int64_t price, int64_t quantity);

/*
 * Returns a json representation of the chart, the user of this function
 * must free the resulting data
 * @param {struct security*} sec The security to serialize
 * @param {char**} json Sets *json to the resulting json
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_get_chart(struct security *sec, char **json);

/*
 * Returns a json representation of the analysis
 * done on this securities chart
 * @param {struct security*} sec The security to serialize
 * @param {char**} json Sets *json to the resulting json
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_get_analysis(struct security *sec, char **json);

/*
 * Returns the latest candle of a given security
 * @param {struct security*} sec The security to serialize
 * @param {char**} json Sets *json to the resulting json
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_get_latest_candle(struct security *sec,
                                                 char **json);

/*
 * Updates the chart given a fixed point number, which must be of the same
 * type given to the order book, and a timestamp in a time unit that is the
 * same as the interval
 * @param {struct security*} sec The security
 * @param {int64_t} price The price
 * @param {uint64_t} ts The timestamp
 * @return {enum RISKI_ERROR_CODE}
 */
enum RISKI_ERROR_CODE security_chart_update(struct security *sec, int64_t price,
                                            uint64_t ts);

/*
 * Frees the security struct
 * @param {struct security**} sec The security to free
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_free(struct security **sec);

/*
 * Gets the hashed index of the security
 * hash is of module 1000
 * @param {char*} n1 The name of the security
 * @param {size_t*} index *index will be set to the bin
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_hash(char *n1, size_t *index);

/*
 * Gets the hash value (index) given a security
 * and not a name
 * @param {struct security*} s The security
 * @param {size_t*} hash The security hash
 */
enum RISKI_ERROR_CODE security_get_hash(struct security *s, size_t *hash);

/*
 * Compares the two security name
 * @param {char*} n1 The name of a security
 * @param {struct security*} n2 The security to compare n1 with
 * @param {bool*} res *res is set to true if n1 is the same name as n2
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE security_cmp(char *n1, struct security *n2, bool *res);

#endif
