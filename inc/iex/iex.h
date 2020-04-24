#ifndef IEX_
#define IEX_

// tcp ip / wireshark / packet analyzer
#include <arpa/inet.h>


#define u_char unsigned char
#include <net/ethernet.h>
#undef u_char

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <pcap.h>


// on interup
#include <signal.h>

// std
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// iex packet and type data
#include <exchange/exchange.h>
#include <iex/packet.h>
#include <iex/types.h>
#include <security/security.h>

// the error codes and stack tracer
#include <error_codes.h>
#include <logger.h>
#include <tracer.h>

/**
 * Processes the IEX Deep data feed
 * @param {char*} file A location to a pcap file provded by IEX
 */
enum RISKI_ERROR_CODE iex_parse_deep(char* file);

/**
 * Represents the IEX exchange
 */
struct exchange* iex_exchange;

/**
 * Stops processing
 */
enum RISKI_ERROR_CODE iex_stop_parse();

/**
 * Must be set to 1 on interrup
 * otherwise will exit with error
 */
int IEX_SIGNAL_INTER;

#endif
