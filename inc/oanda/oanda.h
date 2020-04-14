#ifndef OANDA_
#define OANDA_

#include <arpa/inet.h>
#include <cjson/cjson.h>
#include <error_codes.h>
#include <exchange/exchange.h>
#include <logger.h>
#include <netdb.h>
#include <oanda/request_builder.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <tracer.h>

/*
 * Represents the oanda exchange
 */
struct exchange* exchange_oanda;

/*
 * Connectes to the live oanda feed for forex data
 */
enum RISKI_ERROR_CODE oanda_live(char* token);

#endif
