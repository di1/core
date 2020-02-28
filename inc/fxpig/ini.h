#ifndef FXPIG_INI
#define FXPIG_INI

#include <log/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Represents the [DEFAULT] ini tag
struct fxpig_ini_default {
  char* begin_string;
  char* connection_type;
  int heart_beat_interval;
  int reconnect_interval;
  char* start_time;
  char* end_time;
  char* start_day;
  char* end_day;
  char* data_dictionary;
};

// Represetns a [SESSION] ini tag
struct fxpig_ini_session {
  char* session_qualifier;
  char* sender_comp_id;
  char* target_comp_id;
  char* username;
  char* password;
  char* reset_on_logon;
  char* socket_connection_host;
  int socket_connection_port;
};

// Represetngs the entire INI file
struct fxpig_ini_config {
  struct fxpig_ini_default defaults;
  size_t num_sessions;
  struct fxpig_ini_session* sessions;
};

// Parses an INI file
struct fxpig_ini_config* fxpig_ini_parse(char* file);

// Frees the config file
void fxpig_ini_free(struct fxpig_ini_config** cfg);

#endif
