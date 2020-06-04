#ifndef SERVER_
#define SERVER_

#include <iex/iex.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Werror"
#include <libwebsockets.h>
#pragma clang diagnostic pop

#include <server/message_parser.h>
#include <signal.h>
#include <string.h>

extern int SERVER_INTERRUPTED;

void *server_start(void *);

#endif
