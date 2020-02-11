#include "security/security.h"
#include <server/message_parser.h>

char* init_response(char* security) {

  struct security* sec = exchange_get(iex_exchange, security);

  if (!sec) {
    log_error("%s is not a valid security traded on IEX", security);
    return NULL;
  }
  char* cht = security_get_chart(sec);

  return cht;

}

char* latest_response(char* security) {
  struct security* sec = exchange_get(iex_exchange, security);

  if (!sec) {
    log_error("%s is not a valid security traded on IEX", security);
    return NULL;
  }

  char* cht = security_get_latest_candle(sec);
  return cht;
}

char* parse_message(char* msg, int len) {
  
  // there is some garbage after msg from the websocket
  // so we cut it off in a new array
  char* sanitized_msg = (char*) malloc((len+1)*sizeof(char));
  strncpy(sanitized_msg, msg, len);
  sanitized_msg[len] = '\x0';

  // read the message type
  char* tokened = NULL;
  tokened = strtok(sanitized_msg, "|");

  //log_debug("received event type: %s", tokened);

  if (strcmp("init", tokened) == 0) {
    tokened = strtok(NULL, "|");
    //log_debug("sending full chart for %s", tokened);

    char* response = init_response(tokened);
    free(sanitized_msg);

    return response;
  }
  if (strcmp("latest", tokened) == 0) {
    tokened = strtok(NULL, "|");
    //log_debug("sending latest candle for %s", tokened);

    char* response = latest_response(tokened);
    free(sanitized_msg);

    return response;
  }

  free(sanitized_msg);
  return NULL;
}
