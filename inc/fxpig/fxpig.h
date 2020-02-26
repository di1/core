#ifndef FXPIG_
#define FXPIG_

#include <exchange/exchange.h>
#include <fxpig/ini.h>

/**
 * Entry point for live forex data
 */
void fxpig_live();

struct exchange* fxpig_exchange;

#endif
