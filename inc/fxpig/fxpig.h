#ifndef FXPIG_
#define FXPIG_

#include <arpa/inet.h>
#include <exchange/exchange.h>
#include <fxpig/ini.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct exchange* fxpig_exchange;

/**
 * Entry point for live forex data
 */
void fxpig_live(struct fxpig_ini_config* cfg);

#endif
