#include <fxpig/fxpig.h>

#define FXPIG_SESSION_INDEX 0
#define FIX_DELI "\x01"

#define MessageType "35"
#define Logon "A"
#define Logout "5"
#define Text "58"

#define EncryptionMethod "98"
#define EncryptionMethodUnencrypted "0"
#define HeartBeatInterval "108"
#define ResetSequenceNumberFlag "141"
#define Username "553"
#define Password "554"
#define BeginString "8"
#define BodyLength "9"
#define MsgType "35"
#define SenderCompId "49"
#define TargetCompId "56"
#define MsgSeqNum "34"
#define SendingTime "52"
#define CheckSum "10"
#define BooleanTrue "Y"
#define BooleanFalse "N"

size_t message_number_sent = 0;

// Generates a fix message given a config
// and the parameters of the message body
char* fxpig_generate_fix_message(struct fxpig_ini_config* cfg,
                                 char* message_type, int num_tags, ...) {
  message_number_sent += 1;
  va_list tags;

  // arguments are given like so,
  // (cfg, 2, tag, "value", tag, "value")
  va_start(tags, num_tags);

  time_t rawtime;
  struct tm* info;
  time(&rawtime);
  info = gmtime(&rawtime);

  char timebuf[256] = {0};
  strftime(timebuf, 255, "%Y%m%d-%T", info);

  int initial_size =
      snprintf(NULL, 0,
               MessageType "=%s" FIX_DELI SenderCompId
                           "=%s" FIX_DELI TargetCompId "=%s" FIX_DELI MsgSeqNum
                           "=%lu" FIX_DELI SendingTime "=%s" FIX_DELI,
               message_type, cfg->sessions[FXPIG_SESSION_INDEX].sender_comp_id,
               cfg->sessions[FXPIG_SESSION_INDEX].target_comp_id,
               message_number_sent, timebuf);

  char* msg = (char*)malloc((initial_size + 1) * sizeof(char));
  snprintf(msg, initial_size + 1,
           MessageType "=%s" FIX_DELI SenderCompId "=%s" FIX_DELI TargetCompId
                       "=%s" FIX_DELI MsgSeqNum "=%lu" FIX_DELI SendingTime
                       "=%s" FIX_DELI,
           message_type, cfg->sessions[FXPIG_SESSION_INDEX].sender_comp_id,
           cfg->sessions[FXPIG_SESSION_INDEX].target_comp_id,
           message_number_sent, timebuf);

  for (int i = 0; i < num_tags; ++i) {
    char* tag_id = va_arg(tags, char*);
    char* tag_value = va_arg(tags, char*);
    size_t size = snprintf(NULL, 0, "%s%s=%s" FIX_DELI, msg, tag_id, tag_value);
    msg = realloc(msg, (size + 1) * sizeof(char));
    strcat(msg, tag_id);
    strcat(msg, "=");
    strcat(msg, tag_value);
    strcat(msg, FIX_DELI);
  }
  va_end(tags);

  // append constant header
  int header_size =
      snprintf(NULL, 0, BeginString "=%s" FIX_DELI BodyLength "=%lu" FIX_DELI,
               cfg->defaults.begin_string, strlen(msg));

  char* msg_with_header =
      (char*)malloc((header_size + strlen(msg) + 1) * sizeof(char));
  snprintf(msg_with_header, header_size + 1,
           BeginString "=%s" FIX_DELI BodyLength "=%lu" FIX_DELI,
           cfg->defaults.begin_string, strlen(msg));
  snprintf(msg_with_header + header_size, strlen(msg) + 1, "%s", msg);

  free(msg);

  // append the checksum at the end
  int checksum = 0;
  size_t msg_with_header_len = strlen(msg_with_header);
  for (size_t i = 0; i < msg_with_header_len; ++i) {
    checksum += msg_with_header[i];
  }
  checksum = checksum % 256;

  int checksum_len = snprintf(NULL, 0, CheckSum "=%03d" FIX_DELI, checksum);
  msg_with_header = (char*)realloc(
      msg_with_header, (msg_with_header_len + checksum_len + 1) * sizeof(char));
  snprintf(msg_with_header + msg_with_header_len, checksum_len + 1,
           CheckSum "=%03d" FIX_DELI, checksum);

  return msg_with_header;
}

void fxpig_live(struct fxpig_ini_config* cfg) {
  // connect to socket using tcp

  int sock = 0;
  struct sockaddr_in serv_addr;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    log_error("Socket creation error");
    exit(1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port =
      htons(cfg->sessions[FXPIG_SESSION_INDEX].socket_connection_port);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET,
                cfg->sessions[FXPIG_SESSION_INDEX].socket_connection_host,
                &serv_addr.sin_addr) <= 0) {
    log_error("Invalid address/ Address not supported");
    exit(1);
  }

  if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    log_error("Connection Failed");
    exit(1);
  }

  char* login_message = fxpig_generate_fix_message(
      cfg, Logon, 5, EncryptionMethod, EncryptionMethodUnencrypted,
      HeartBeatInterval, "0", ResetSequenceNumberFlag, BooleanTrue, Username,
      cfg->sessions[FXPIG_SESSION_INDEX].username, Password,
      cfg->sessions[FXPIG_SESSION_INDEX].password);

  // send login message
  int sent_len = send(sock, login_message, strlen(login_message), 0);
  printf("=> (%d) %s\n", sent_len, login_message);
  free(login_message);

  char buff[1024] = {0};
  bool sent_logout = false;

  while (true) {
    int numread;

    if ((numread = read(sock, buff, sizeof(buff) - 1)) == -1) {
      log_error("reading error");
      exit(1);
    }

    if (numread == 0) break;

    buff[numread] = '\x0';

    printf("<= %s\n", buff);

    // send logout message
    if (!sent_logout) {
      char* logout_message = fxpig_generate_fix_message(cfg, Logout, 0);
      sent_len = send(sock, logout_message, strlen(logout_message), 0);
      printf("=> (%d) %s\n", sent_len, logout_message);
      sent_logout = true;
      free(logout_message);
    }
  }
  printf("server closed connection");

  close(sock);
}
