#include <logger/logger.h>

char* hostname() {
  FILE* fp = fopen("/etc/hostname", "r");
  if (!fp) {
    printf("error: /etc/hostname does not exist\n");
    exit(1);
  }
  return "fdjskla";
}

void printf_log(const char* lvl, const char* fmt, ...) {  
  // retreive the hostname one time
  static char* host_name = NULL;
  if (!host_name)
    host_name = hostname();
  
  // print out the log level followed by the hostname
  printf("%s %s ", lvl, host_name);

  // print out the message the user wants
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  // implimentation will define the new line
  printf("\n");
}
