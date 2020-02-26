#include <fxpig/fxpig.h>

#define FXPIG_SESSION_INDEX 0
#define FIX_DELI "\x01"

struct fxpig_fix_header {
  // 8
  const char* begin_string;  // represents the type of fix engine
  // 9
  size_t body_length;  // the length of the message body after the body tag
  // 35
  char msg_type[3];  // the message type is at max 2 character + NULL
  // 49
  const char*
      sender_comp_id;  // Assigned value used to identify firm sending message
  // 56
  const char*
      target_comp_id;  // Assigned value used to identify receiving firm.
  // 34
  int msg_seq_num;  // Message seq number
  // 52
  char sending_time[18];  // the utc timestamp
};

struct fxpig_fix_logon {
  // 98
  int encryption_methods;  // always 0
  // 108
  int heart_beat_interval;  // 30? (can put 0 for no heartbeat)
  // 141
  char reset_sequence_number_flag;  // always set this to Y
  // 553
  char* username;  // username
  // 554
  char* password;  // password
};

struct fxpig_fix_trailer {
  int checksum;  // three byte simple checksum
};

// the message header doesn't change much so we can reuse 1 template
// with only modifing 3 variables
struct fxpig_fix_header header_template;

void fxpig_fix_header_set_time() {
  time_t rawtime;
  struct tm* info;
  time(&rawtime);
  info = gmtime(&rawtime);
  strftime(header_template.sending_time, sizeof(header_template.sending_time),
           "%Y%M%d-%H:%M:%S", info);
}

unsigned char fxpig_fix_compute_checksum(char* buff, ssize_t len) {
  int sum = 0;
  for (ssize_t i = 0; i < len; ++i) {
    sum += buff[i];
  }
  return sum % 256;
}

char* fxpig_fix_finalize_message(ssize_t message_size, char* buff) {
  ssize_t added_header_length =
      snprintf(NULL, 0, "8=%s" FIX_DELI "9=%ld" FIX_DELI "%s",
               header_template.begin_string, message_size, buff);

  char* msg_with_buf = (char*)malloc((added_header_length + 1) * sizeof(char));
  snprintf(msg_with_buf, added_header_length + 1,
           "8=%s" FIX_DELI "9=%ld" FIX_DELI "%s", header_template.begin_string,
           message_size, buff);

  // printf("%s\n", msg_with_buf);

  unsigned char checksum =
      fxpig_fix_compute_checksum(msg_with_buf, added_header_length);
  ssize_t final_message_length =
      snprintf(NULL, 0, "%s10=%03u" FIX_DELI, msg_with_buf, checksum);

  char* final_message =
      (char*)malloc((final_message_length + 1) * sizeof(char));
  snprintf(final_message, final_message_length + 1, "%s10=%03u" FIX_DELI,
           msg_with_buf, checksum);

  free(buff);
  free(msg_with_buf);
  return final_message;
}

// Generates a login message
char* fxpig_fix_construct_logon(struct fxpig_ini_config* config) {
  struct fxpig_fix_logon logon_message;

  // set to current utc time
  fxpig_fix_header_set_time();

  // incease the msg count
  header_template.msg_seq_num += 1;

  // set the message type
  header_template.msg_type[0] = 'A';
  header_template.msg_type[1] = '\x0';

  // setup the login message
  logon_message.encryption_methods = 0;
  logon_message.heart_beat_interval = 0;
  logon_message.reset_sequence_number_flag = 'Y';
  logon_message.username = config->sessions[FXPIG_SESSION_INDEX].username;
  logon_message.password = config->sessions[FXPIG_SESSION_INDEX].password;

  // compute the message size not including begin_string
  ssize_t message_size = snprintf(
      NULL, 0,
      "35=%s" FIX_DELI "49=%s" FIX_DELI "56=%s" FIX_DELI "34=%d" FIX_DELI
      "52=%s" FIX_DELI "98=%d" FIX_DELI "108=%d" FIX_DELI "141=%c" FIX_DELI
      "553=%s" FIX_DELI "554=%s" FIX_DELI,
      header_template.msg_type, header_template.sender_comp_id,
      header_template.target_comp_id, header_template.msg_seq_num,
      header_template.sending_time, logon_message.encryption_methods,
      logon_message.heart_beat_interval,
      logon_message.reset_sequence_number_flag, logon_message.username,
      logon_message.password);

  char* buff = (char*)malloc((message_size + 1) * sizeof(char));
  snprintf(buff, message_size + 1,
           "35=%s" FIX_DELI "49=%s" FIX_DELI "56=%s" FIX_DELI "34=%d" FIX_DELI
           "52=%s" FIX_DELI "98=%d" FIX_DELI "108=%d" FIX_DELI "141=%c" FIX_DELI
           "553=%s" FIX_DELI "554=%s" FIX_DELI,
           header_template.msg_type, header_template.sender_comp_id,
           header_template.target_comp_id, header_template.msg_seq_num,
           header_template.sending_time, logon_message.encryption_methods,
           logon_message.heart_beat_interval,
           logon_message.reset_sequence_number_flag, logon_message.username,
           logon_message.password);

  char* final_message = fxpig_fix_finalize_message(message_size, buff);
  printf("%s\n", final_message);

  return NULL;
}

void fxpig_live(struct fxpig_ini_config* config) {
  header_template.begin_string = config->defaults.begin_string;
  header_template.sender_comp_id =
      config->sessions[FXPIG_SESSION_INDEX].sender_comp_id;
  header_template.target_comp_id =
      config->sessions[FXPIG_SESSION_INDEX].target_comp_id;
  header_template.msg_seq_num = 0;
  fxpig_fix_construct_logon(config);
}
