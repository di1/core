#ifndef IEX_
#define IEX_

#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>
#include <log/log.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// iex packet and type data
#include <security/security.h>
#include <exchange/exchange.h>

/**
 * Processes the IEX Deep data feed
 *
 * @param file A location to a pcap file provded by IEX
 */
void iex_parse_deep(char* file);

/**
 * Represents the IEX exchange
 */
struct exchange* iex_exchange;

/**
 * Stops processing
 */
void iex_stop_parse();

/**
 * Must be set to 1 on interrup
 * otherwise will exit 1
 */
int IEX_SIGNAL_INTER;

#endif
