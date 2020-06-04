#ifndef SERVER_
#define SERVER_

#include <iex/iex.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wduplicate-enum"
#include <libwebsockets.h>
#pragma clang diagnostic pop

#include <server/message_parser.h>
#include <signal.h>
#include <string.h>

extern int SERVER_INTERRUPTED;

void *server_start(void *);

#endif
