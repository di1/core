#ifndef MESSAGE_PARSER_
#define MESSAGE_PARSER_

#include <exchange/exchange.h>
#include <iex/iex.h>
#include <security/search.h>
#include <log/log.h>
#include <stdlib.h>
#include <string.h>

/**
 * Takes in a message
 * @param {char*} msg The message to parse
 * @param {int} len The length of the message
 * @param {char**} resp Will set *resp to the message to send back
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE parse_message(char* msg, int len, char** resp);

#endif
