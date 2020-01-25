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
 * Creates a new security and puts in the hashtable,
 * Name and interval definitions are equivelent to security_new
 * and can be found in security.h
 */
void exchange_put(struct exchange* e, char* name, uint64_t interval);

/**
 * Gets a security given its name
 * Returns null if the security does not exist in the exchange
 */
struct security* exchange_get(struct exchange* e, char* name);

/**
 * Frees the exchange and all the securities that were added to it
 */
void exchange_free(struct exchange** e);

/**
 * Runs tests on the exchange
 */
void test_exchange();

#endif
