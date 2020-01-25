#ifndef EXCHANGE_
#define EXCHANGE_

#include <security/security.h>

struct exchange;

/**
 * Creates a new exchange with a given name
 * This will create a copy of the name
 */
struct exchange* exchange_new(char* name);

/**
 * Puts a security in the exchange
 */
void exchange_put(struct security* s);

/**
 * Gets a security given its name
 * Returns null if the security does not exist in the exchange
 */
struct exchange* exchange_get(char* name);

/**
 * Frees the exchange and all the securities that were added to it
 */
void exchange_free(struct exchange** e);

/**
 * Runs tests on the exchange
 */
void exchange_test();

#endif
