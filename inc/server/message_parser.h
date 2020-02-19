#ifndef MESSAGE_PARSER_
#define MESSAGE_PARSER_

#include <exchange/exchange.h>
#include <iex/iex.h>
#include <log/log.h>
#include <stdlib.h>
#include <string.h>

/**
 * Takes in a message
 */
char* parse_message(char* msg, int len);

#endif
