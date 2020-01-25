#ifndef SECURITY_
#define SECURITY_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <book/book.h>
#include <chart/chart.h>
#include <log/log.h>

#define SECURITY_INTERVAL_MINUTE_NANOSECONDS 1e+9
#define SECURITY_HASH_MODULE_VAL 500

/**
 * Private definition of a security
 */
struct security;

/**
 * Creates a new security given a name.
 * A security consists of an order book and a
 * chart and is identified by its name.
 *
 * the name given will be copied to the heap, the
 * security will own its own copy of the name data
 *
 * The interval should be the spacing between candles
 * in a given time unit. For example if the time unit is nano seconds
 * and 1 minute candles are wanted, an interval of 1e+9 should be used.
 */
struct security* security_new(char* name, uint64_t interval);

/**
 * Updates the order book associated with the security.
 * Side, true, is for the buy side and false is the sell side
 * The price is a a fixed point number. As long as all the data
 * is the same fixed point number this function will perform correctly.
 * You can not mix a fixed 4 decimal number with a fixed 5 decimal number,
 * if done this will result in undefined behavior. The quantity is the number
 * of shares
 */
void security_book_update(struct security* sec, bool side, int64_t price,
    int64_t quantity);

/**
 * Updates the chart given a fixed point number, which must be of the same
 * type given to the order book, and a timestamp in a time unit that is the
 * same as the interval
 */
void security_chart_update(struct security* sec, int64_t price, uint64_t ts);

/**
 * Frees the security struct
 */
void security_free(struct security** sec);

/**
 * Gets the hashed index of the security
 * hash is of module 1000
 */
size_t security_hash(char* n1);

/**
 * Compares the two security name
 */
bool security_cmp(char* n1, char* n2);

/**
 * The test security
 */
void test_security();

#endif
