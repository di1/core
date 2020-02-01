#ifndef MESSAGE_PARSER_
#define MESSAGE_PARSER_

#include <stdlib.h>
#include <string.h>

#include <log/log.h>
#include <exchange/exchange.h>
#include <iex/iex.h>

/**
 * Takes in a message
 */
char* parse_message(char* msg, int len);

#endif
