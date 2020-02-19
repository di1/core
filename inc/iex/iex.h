#ifndef IEX_
#define IEX_

#include <arpa/inet.h>
#include <log/log.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <pcap.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// iex packet and type data
#include <exchange/exchange.h>
#include <security/security.h>

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
