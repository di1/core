#include "cjson/cjson.h"
#include <oanda/oanda.h>

static char *oanda_working_account = NULL;
static char **oanda_tradeble_instruments = NULL;

struct exchange *exchange_oanda = NULL;

static enum RISKI_ERROR_CODE oanda_connect(int *ret) {
  int sockfd;
  struct sockaddr_in servaddr;

  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  }
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("198.105.27.154");
  servaddr.sin_port = htons(443);

  // connect the client socket to server socket
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    exit(0);
  } else {
    logger_info(__func__, FILENAME_SHORT, __LINE__,
                "connected to oanda server");
  }

  *ret = sockfd;

  return RISKI_ERROR_CODE_NONE;
}

static void SSL_read_line(SSL *conn, char **buff) {
  size_t line_length = 0;
  char *line = NULL;

  char c = '\x0';
  int ssl_err;
  while ((ssl_err = SSL_read(conn, &c, 1)) && c != '\n') {
    switch (SSL_get_error(conn, ssl_err)) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      break;
    case SSL_ERROR_ZERO_RETURN:
      continue;
    default:
      printf("SSL_read_line error\n");
      exit(1);
    }
    line_length += 1;
    line = (char *)realloc(line, line_length * sizeof(char));
    line[line_length - 1] = c;
  }

  line_length += 1;
  line = (char *)realloc(line, line_length * sizeof(char));
  line[line_length - 1] = '\x0';

  *buff = line;
}

static void SSL_read_http_header(SSL *conn, int *content_length) {
  char *line = NULL;
  while (1) {
    SSL_read_line(conn, &line);
    if (strncmp(line, "Content-Length", 14) == 0) {
      char *tok = NULL;
      tok = strtok(line, " ");
      tok = strtok(NULL, " ");
      *content_length = atoi(tok);
    } else if (strcmp(line, "\r") == 0) {
      break;
    }
    free(line);
  }
  free(line);
}

static void SSL_http_request_json(SSL *conn, char *request_body,
                                  cJSON **response) {
  SSL_write(conn, request_body, (int)strlen(request_body));

  int content_length = 0;
  SSL_read_http_header(conn, &content_length);

  char *response_body = NULL;
  response_body = (char *)malloc((uint64_t)(content_length + 1) * sizeof(char));

  int total_read = 0;
  while (total_read < content_length) {
    total_read += SSL_read(conn, &response_body[total_read], content_length);
  }
  response_body[content_length] = '\x0';

  cJSON *response_json = cJSON_Parse(response_body);
  free(response_body);

  *response = response_json;
}

static enum RISKI_ERROR_CODE oanda_main_loop(SSL *conn,
                                             char *pricing_request_body) {
  long start_time = time(NULL);
  int requests_per_second = 0;
  while (1) {
    cJSON *price_update_json = NULL;
    SSL_http_request_json(conn, pricing_request_body, &price_update_json);

    int num_prices = 0;
    const cJSON *prices_json = cJSON_GetObjectItem(price_update_json, "prices");
    num_prices = cJSON_GetArraySize(prices_json);

    for (int i = 0; i < num_prices; ++i) {
      const cJSON *instrument_price_data = cJSON_GetArrayItem(prices_json, i);
      cJSON *instrument_name =
          cJSON_GetObjectItem(instrument_price_data, "instrument");
      cJSON *instrument_bids =
          cJSON_GetObjectItem(instrument_price_data, "bids");
      cJSON *instrument_asks =
          cJSON_GetObjectItem(instrument_price_data, "asks");
      cJSON *instrument_closeout_bid =
          cJSON_GetObjectItem(instrument_price_data, "closeoutBid");
      cJSON *instrument_closeout_ask =
          cJSON_GetObjectItem(instrument_price_data, "closeoutAsk");

      char *instrument_name_str = cJSON_GetStringValue(instrument_name);

      int num_bids = cJSON_GetArraySize(instrument_bids);
      int num_asks = cJSON_GetArraySize(instrument_asks);

      char *bid_str = NULL;
      char *ask_str = NULL;

      if (num_bids != 0) {
        const cJSON *instrument_best_bid =
            cJSON_GetArrayItem(instrument_bids, 0);
        cJSON *bid_object = cJSON_GetObjectItem(instrument_best_bid, "price");
        bid_str = cJSON_GetStringValue(bid_object);
      } else {
        logger_analysis(instrument_name_str, "FEED", __func__, FILENAME_SHORT,
                        __LINE__, "forced to use closeout bid");
        bid_str = cJSON_GetStringValue(instrument_closeout_bid);
      }

      if (num_asks != 0) {
        const cJSON *instrument_best_ask =
            cJSON_GetArrayItem(instrument_asks, 0);
        cJSON *ask_object = cJSON_GetObjectItem(instrument_best_ask, "price");
        ask_str = cJSON_GetStringValue(ask_object);
      } else {
        logger_analysis(instrument_name_str, "FEED", __func__, FILENAME_SHORT,
                        __LINE__, "forced to use closeout ask");
        bid_str = cJSON_GetStringValue(instrument_closeout_ask);
      }

      // Find the . index in the str for bid
      size_t idxToDel = 0;
      while (bid_str[idxToDel] != '.')
        idxToDel += 1;
      memmove(&bid_str[idxToDel], &bid_str[idxToDel + 1],
              strlen(bid_str) - idxToDel);
      int64_t bid = (int64_t)atoi(bid_str);

      // Do the same for the ask
      idxToDel = 0;
      while (ask_str[idxToDel] != '.')
        idxToDel += 1;
      memmove(&ask_str[idxToDel], &ask_str[idxToDel + 1],
              strlen(ask_str) - idxToDel);
      int64_t ask = (int64_t)atoi(ask_str);

      struct security *sec = NULL;
      TRACE(exchange_get(exchange_oanda, instrument_name_str, &sec));
      cJSON *ts = cJSON_GetObjectItem(instrument_price_data, "time");

      char *ts_str = cJSON_GetStringValue(ts);
      idxToDel = 0;
      while (ts_str[idxToDel] != '.')
        idxToDel += 1;
      memmove(&ts_str[idxToDel], &ts_str[idxToDel + 1],
              strlen(ts_str) - idxToDel);
      uint64_t ts_nanosecond = strtoull(ts_str, NULL, 10);
      TRACE(security_chart_update(sec, bid, bid, ask, ts_nanosecond));
    }
    cJSON_Delete(price_update_json);

    if (time(NULL) - start_time >= 1) {
      requests_per_second = 0;
      start_time = time(NULL);
    } else {
      requests_per_second += 1;
    }
  }
}

enum RISKI_ERROR_CODE oanda_live(char *token) {
  TRACE(exchange_new("OANDA", &exchange_oanda));

  logger_info(__func__, FILENAME_SHORT, __LINE__, "using oanda api token %s",
              token);

  int socket = 0;
  TRACE(oanda_connect(&socket));

  if (socket == 0) {
    return RISKI_ERROR_CODE_UNKNOWN;
  }

  // init ssl
  SSL_load_error_strings();
  SSL_library_init();
  SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());

  // create an SSL connection and attach it to the socket
  SSL *conn = SSL_new(ssl_ctx);
  SSL_set_fd(conn, socket);

  // perform the SSL/TLS handshake with the server - when on the
  // server side, this would use SSL_accept()

  if (!X509_VERIFY_PARAM_set1_host(SSL_get0_param(conn),
                                   "api-fxpractice.oanda.com", 0))
    abort();
  int err = SSL_connect(conn);
  if (err <= 0) {
    int errcode = SSL_get_error(conn, err);
    switch (errcode) {
    case SSL_ERROR_NONE:
      break; // Cannot happen if err <=0
    case SSL_ERROR_ZERO_RETURN:
      fprintf(stderr, "SSL connect returned 0.");
      break;
    case SSL_ERROR_WANT_READ:
      fprintf(stderr, "SSL connect: Read Error.");
      break;
    case SSL_ERROR_WANT_WRITE:
      fprintf(stderr, "SSL connect: Write Error.");
      break;
    case SSL_ERROR_WANT_CONNECT:
      fprintf(stderr, "SSL connect: Error connect.");
      break;
    case SSL_ERROR_WANT_ACCEPT:
      fprintf(stderr, "SSL connect: Error accept.");
      break;
    case SSL_ERROR_WANT_X509_LOOKUP:
      fprintf(stderr, "SSL connect error: X509 lookup.");
      break;
    case SSL_ERROR_SYSCALL:
      fprintf(stderr, "SSL connect: Error in system call.");
      break;
    case SSL_ERROR_SSL:
      fprintf(stderr, "SSL connect: Protocol Error.");
      break;
    default:
      fprintf(stderr, "Failed SSL connect.");
    }
    exit(0);
  }

  /*
   * Get the working account id
   */
  cJSON *account_json = NULL;
  char *http_get_accounts = NULL;
  TRACE(oanda_v20_v3_accounts("api-fxpractice.oanda.com", token,
                              &http_get_accounts));
  SSL_http_request_json(conn, http_get_accounts, &account_json);
  const cJSON *accounts = cJSON_GetObjectItem(account_json, "accounts");
  for (int i = 0; i < cJSON_GetArraySize(accounts); ++i) {
    const cJSON *account = cJSON_GetArrayItem(accounts, i);
    cJSON *_id = cJSON_GetObjectItem(account, "id");
    const char *id_str = cJSON_GetStringValue(_id);
    oanda_working_account = strdup(id_str);
  }
  printf("%s\n", cJSON_Print(account_json));
  free(http_get_accounts);

  cJSON_Delete(account_json);
  logger_info(__func__, FILENAME_SHORT, __LINE__, "oanda account id: %s",
              oanda_working_account);

  /*
   * Read the tradable security for this account from the oanda server
   */
  char *http_get_account_instruments = NULL;
  cJSON *instruments_json = NULL;
  TRACE(oanda_v20_v3_accounts_instruments("api-fxpractice.oanda.com", token,
                                          &http_get_account_instruments,
                                          oanda_working_account));

  SSL_http_request_json(conn, http_get_account_instruments, &instruments_json);
  free(http_get_account_instruments);

  const cJSON *instruments_array =
      cJSON_GetObjectItem(instruments_json, "instruments");

  int num_instruments = cJSON_GetArraySize(instruments_array);
  oanda_tradeble_instruments =
      (char **)malloc((uint64_t)num_instruments * sizeof(char *));

  for (int i = 0; i < num_instruments; ++i) {
    const cJSON *instrument = cJSON_GetArrayItem(instruments_array, i);
    cJSON *displayName = cJSON_GetObjectItem(instrument, "name");
    oanda_tradeble_instruments[i] = strdup(cJSON_GetStringValue(displayName));
    cJSON *precision_json = cJSON_GetObjectItem(instrument, "displayPrecision");
    int precision = (int)(cJSON_GetNumberValue(precision_json));
    TRACE(exchange_put(exchange_oanda, oanda_tradeble_instruments[i],
                       SECURITY_INTERVAL_MINUTE_NANOSECONDS, precision));
    // get the candles we have missed
  }
  cJSON_Delete(instruments_json);

  /*
   * Start the main loop requesting the latest updates
   */
  char *pricing_request_body = NULL;
  TRACE(oanda_v20_v3_accounts_pricing(
      "api-fxpractice.oanda.com", token, oanda_tradeble_instruments,
      num_instruments, oanda_working_account, &pricing_request_body));

  oanda_main_loop(conn, pricing_request_body);

  SSL_shutdown(conn);
  SSL_free(conn);
  SSL_CTX_free(ssl_ctx);
  close(socket);
  free(oanda_working_account);
  return RISKI_ERROR_CODE_NONE;
}
