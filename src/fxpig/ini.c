#include <fxpig/ini.h>
#include <stdio.h>

#include "log/log.h"

/**
 * Reads up to \n and returns the data up to \n incrementing the fp
 * 1 character past the \n character
 */
char* file_read_line(FILE* fp) {
  int line_length = 0;
  char c;
  while ((c = fgetc(fp)) && (c != EOF)) {
    line_length += 1;
    if (c == '\n') break;
  }
  if (c == EOF) return NULL;
  char* line = (char*)malloc((line_length + 1) * sizeof(char));
  line[line_length - 1] = '\x0';
  fseek(fp, (-1) * line_length, SEEK_CUR);

  for (int i = 0; i < line_length - 1; ++i) {
    line[i] = fgetc(fp);
  }
  fgetc(fp);

  return line;
}

void fxpig_init_config(struct fxpig_ini_config* config) {
  config->sessions = NULL;
  config->num_sessions = 0;
  config->defaults.begin_string = NULL;
  config->defaults.connection_type = NULL;
  config->defaults.data_dictionary = NULL;
  config->defaults.end_day = NULL;
  config->defaults.end_time = NULL;
  config->defaults.heart_beat_interval = 0;
  config->defaults.reconnect_interval = 0;
  config->defaults.start_day = NULL;
  config->defaults.start_time = NULL;
}

struct fxpig_ini_config* fxpig_ini_parse(char* file) {
  log_debug("parsing ini file %s", file);
  FILE* fp = fopen(file, "r");

  if (fp == NULL) {
    log_error("%s does not exist", file);
  }

  // initalize the config
  struct fxpig_ini_config* config =
      (struct fxpig_ini_config*)malloc(sizeof(struct fxpig_ini_config));
  fxpig_init_config(config);

  char* line = NULL;
  bool inSession = false;
  while ((line = file_read_line(fp)) && line != NULL) {
    if (strcmp(line, "[DEFAULT]") == 0) {
      inSession = false;
      continue;
    } else if (strcmp(line, "[SESSION]") == 0) {
      inSession = true;
      config->num_sessions += 1;
      config->sessions =
          realloc(config->sessions,
                  sizeof(struct fxpig_ini_session) * config->num_sessions);
      continue;
    } else if (strcmp(line, "\x0") == 0) {
      continue;
    }
    char* id = strtok(line, "=");
    char* value = strtok(NULL, "=");
    if (!inSession) {
      if (strcmp(id, "BeginString") == 0) {
        config->defaults.begin_string = value;
      } else if (strcmp(id, "ConnectionType") == 0) {
        config->defaults.connection_type = value;
      } else if (strcmp(id, "HeartBtInt") == 0) {
        config->defaults.heart_beat_interval = atoi(value);
      } else if (strcmp(id, "ReconnectInterval") == 0) {
        config->defaults.reconnect_interval = atoi(value);
      } else if (strcmp(id, "StartTime") == 0) {
        config->defaults.start_time = value;
      } else if (strcmp(id, "EndTime") == 0) {
        config->defaults.end_time = value;
      } else if (strcmp(id, "StartDay") == 0) {
        config->defaults.start_day = value;
      } else if (strcmp(id, "EndDay") == 0) {
        config->defaults.end_day = value;
      } else if (strcmp(id, "DataDictionary") == 0) {
        config->defaults.data_dictionary = value;
      } else {
        log_error("unknown id=%s with value=%s", id, value);
        exit(1);
      }
    } else {
      if (strcmp(id, "SessionQualifier") == 0) {
        config->sessions[config->num_sessions - 1].session_qualifier = value;
      } else if (strcmp(id, "SenderCompID") == 0) {
        config->sessions[config->num_sessions - 1].sender_comp_id = value;
      } else if (strcmp(id, "TargetCompID") == 0) {
        config->sessions[config->num_sessions - 1].target_comp_id = value;
      } else if (strcmp(id, "Username") == 0) {
        config->sessions[config->num_sessions - 1].username = value;
      } else if (strcmp(id, "Password") == 0) {
        config->sessions[config->num_sessions - 1].password = value;
      } else if (strcmp(id, "ResetOnLogon") == 0) {
        config->sessions[config->num_sessions - 1].reset_on_logon = value;
      } else if (strcmp(id, "SocketConnectHost") == 0) {
        config->sessions[config->num_sessions - 1].socket_connection_host =
            value;
      } else if (strcmp(id, "SocketConnectPort") == 0) {
        config->sessions[config->num_sessions - 1].socket_connection_port =
            atoi(value);
      } else {
        log_error("unknown id=%s with value=%s", id, value);
        exit(1);
      }
    }
  }
  return config;
}
