#include <server/message_parser.h>

#include "error_codes.h"
#include "security/security.h"

enum RISKI_ERROR_CODE init_response(char* security, char** resp) {
  PTR_CHECK(security, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(resp, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct security* sec = NULL;
  TRACE(exchange_get(iex_exchange, security, &sec));

  if (!sec) {
    log_error("%s is not a valid security traded on IEX", security);
    return RISKI_ERROR_CODE_INVALID_SYMBOL;
  }
  char* cht = NULL;
  TRACE(security_get_chart(sec, &cht));
  *resp = cht;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE latest_response(char* security, char** resp) {
  PTR_CHECK(security, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(resp, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct security* sec = NULL;
  TRACE(exchange_get(iex_exchange, security, &sec));

  if (!sec) {
    log_error("%s is not a valid security traded on IEX", security);
    return RISKI_ERROR_CODE_INVALID_SYMBOL;
  }

  char* cht = NULL;

  TRACE(security_get_latest_candle(sec, &cht));
  *resp = cht;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE analysis_response(char* security, char** resp) {
  PTR_CHECK(security, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(resp, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct security* sec = NULL;
  TRACE(exchange_get(iex_exchange, security, &sec));

  if (!sec) {
    log_error("%s is not a valid secuiryt traded on IEX", security);
    return RISKI_ERROR_CODE_INVALID_SYMBOL;
  }

  char* analysis_json = NULL;
  TRACE(security_get_analysis(sec, &analysis_json));

  *resp = analysis_json;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE parse_message(char* msg, int len, char** resp) {

  PTR_CHECK(msg, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(msg, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // there is some garbage after msg from the websocket
  // so we cut it off in a new array
  char* sanitized_msg = (char*)malloc((len + 1) * sizeof(char));
  PTR_CHECK(sanitized_msg, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  strncpy(sanitized_msg, msg, len);
  sanitized_msg[len] = '\x0';

  // read the message type
  char* tokened = NULL;
  tokened = strtok(sanitized_msg, "|");

  // log_debug("received event type: %s", tokened);

  char* response = NULL;
  if (strcmp("init", tokened) == 0) {
    tokened = strtok(NULL, "|");

    TRACE(init_response(tokened, &response));
    free(sanitized_msg);
  } else if (strcmp("latest", tokened) == 0) {
    tokened = strtok(NULL, "|");

    TRACE(latest_response(tokened, &response));
    free(sanitized_msg);
  } else if (strcmp("analysis", tokened) == 0) {
    tokened = strtok(NULL, "|");

    TRACE(analysis_response(tokened,&response));
    free(sanitized_msg);
  } else {
    free(sanitized_msg);
    return RISKI_ERROR_CODE_INVALID_REQUEST;
    free(sanitized_msg);
  }

  *resp = response;
  return RISKI_ERROR_CODE_NONE;
}
