#ifndef SERVER_
#define SERVER_

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

#include "message_parser.h"

void* server_start(void*);

#endif
