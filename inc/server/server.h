#ifndef SERVER_
#define SERVER_

#include <libwebsockets.h>
#include <signal.h>
#include <string.h>

#include "message_parser.h"

extern int SERVER_INTERRUPTED;

void* server_start(void*);

#endif
