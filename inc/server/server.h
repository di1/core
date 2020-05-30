#ifndef SERVER_
#define SERVER_

#include <iex/iex.h>
#include <libwebsockets.h>
#include <server/message_parser.h>
#include <signal.h>
#include <string.h>

extern int SERVER_INTERRUPTED;

void *server_start (void *);

#endif
