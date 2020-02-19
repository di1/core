/**
 * Defines an order book
 */
#ifndef BOOK_
#define BOOK_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define BUY_SIDE true
#define SELL_SIDE false

/**
 * Export the private book class
 * to avoid void* in future code
 */
struct book;

/**
 * Creates a new book
 */
struct book* book_new();

/**
 * Updates the buy/sell side of the order book
 * given a fixed point number (price) and the
 * quantity of the transaction. A quantity of 0
 * will delete the level, price, from the order book.
 * Side should either be BUY_SIDE or SELL_SIDE
 */
void book_update(bool side, struct book* t, int64_t price, int64_t quantity);

/**
 * Used to correctly free a book
 */
void book_free(struct book** t);

/**
 * Runs test on book
 */
void test_book();

#endif
