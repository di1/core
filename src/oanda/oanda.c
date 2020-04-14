#include <oanda/oanda.h>

char* oanda_working_account = NULL;

enum RISKI_ERROR_CODE oanda_connect(int* ret) {
  int sockfd;
  struct sockaddr_in servaddr;

  // socket create and varification
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
  if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
    exit(0);
  } else {
    logger_info(__func__, __FILENAME__, __LINE__, "connected to oanda server");
  }

  *ret = sockfd;

  return RISKI_ERROR_CODE_NONE;
}

void SSL_read_line(SSL* conn, char** buff) {
  size_t line_length = 0;
  char* line = NULL;

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
    line = (char*)realloc(line, line_length * sizeof(char));
    line[line_length - 1] = c;
  }

  line_length += 1;
  line = (char*)realloc(line, line_length * sizeof(char));
  line[line_length - 1] = '\x0';

  *buff = line;
}

void SSL_read_http_header(SSL* conn, size_t* content_length) {
  char* line = NULL;
  while (1) {
    SSL_read_line(conn, &line);
    if (strncmp(line, "Content-Length", 14) == 0) {
      char* tok = NULL;
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

enum RISKI_ERROR_CODE oanda_live(char* token) {
  logger_info(__func__, __FILENAME__, __LINE__, "using oanda api token %s",
              token);

  int socket = 0;
  TRACE(oanda_connect(&socket));

  if (socket == 0) {
    return RISKI_ERROR_CODE_UNKNOWN;
  }

  // init ssl
  SSL_load_error_strings();
  SSL_library_init();
  SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_client_method());

  // create an SSL connection and attach it to the socket
  SSL* conn = SSL_new(ssl_ctx);
  SSL_set_fd(conn, socket);

  // perform the SSL/TLS handshake with the server - when on the
  // server side, this would use SSL_accept()
  int err = SSL_connect(conn);
  if (err != 1) abort();  // handle error

  // Get the working account ID
  char* get_accounts = NULL;
  TRACE(
      oanda_v20_v3_accounts("api-fxpractice.oanda.com", token, &get_accounts));

  SSL_write(conn, get_accounts, strlen(get_accounts));
  free(get_accounts);

  size_t content_length = 0;
  SSL_read_http_header(conn, &content_length);

  char* account_response = NULL;
  account_response = (char*)malloc((content_length + 1) * sizeof(char));
  SSL_read(conn, account_response, content_length);
  account_response[content_length] = '\x0';

  cJSON* account_json = cJSON_Parse(account_response);
  const cJSON* accounts = cJSON_GetObjectItem(account_json, "accounts");
  for (size_t i = 0; i < (size_t)cJSON_GetArraySize(accounts); ++i) {
    const cJSON* account = cJSON_GetArrayItem(accounts, i);
    cJSON* _id = cJSON_GetObjectItem(account, "id");
    const char* id_str = cJSON_GetStringValue(_id);
    oanda_working_account = strdup(id_str);
  }
  cJSON_Delete(account_json);

  logger_info(__func__, __FILENAME__, __LINE__, "oanda account id: %s",
              oanda_working_account);

  SSL_shutdown(conn);
  SSL_free(conn);
  SSL_CTX_free(ssl_ctx);
  close(socket);
  free(oanda_working_account);
  return RISKI_ERROR_CODE_NONE;
}
