#ifndef MESSAGE_PARSER_
#define MESSAGE_PARSER_

#include <exchange/exchange.h>
#include <iex/iex.h>
#include <logger.h>
#include <oanda/oanda.h>
#include <security/search.h>
#include <stdlib.h>
#include <string.h>

/**
 * Takes in a message
 * @param msg The message to parse
 * @param len The length of the message
 * @param resp Will set *resp to the message to send back
 * @return The status
 */
enum RISKI_ERROR_CODE parse_message(char *msg, size_t len, char **resp);

#endif
