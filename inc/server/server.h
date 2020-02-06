#ifndef SERVER_
#define SERVER_

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

#include "message_parser.h"

extern int SERVER_INTERRUPTED;

void* server_start(void*);

#endif
