#ifndef EXCHANGE_
#define EXCHANGE_

#include <security/security.h>

/*
 * Private struct describing an exchange
 */
struct exchange;

/*
 * Creates a new exchange with a given name
 * This will create a copy of the name
 * @param {char*} name The name of the security
 * @param {struct exchange**} exchange Will set *exchange to the new exchange
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE exchange_new(char *name, struct exchange **exchange);

/*
 * Creates a new security and puts in the hashtable,
 * Name and interval definitions are equivelent to security_new
 * and can be found in security.h
 */
enum RISKI_ERROR_CODE exchange_put(struct exchange *e, char *name,
                                   uint64_t interval);

/*
 * Gets a security given its name
 * Returns null if the security does not exist in the exchange
 * @param {struct exchange*} e The exchange to get a security from
 * @param {char*} name The name of the security
 * @param {struct security**} sec Will set *sec to the security
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE exchange_get(struct exchange *e, char *name,
                                   struct security **sec);

/*
 * Frees the exchange and all the securities that were added to it
 * @param {struct exchange**} e Will free *e and set *e to NULL
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE exchange_free(struct exchange **e);
#endif
