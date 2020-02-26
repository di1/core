#include <fxpig/fix.h>

struct fix_default_settings {
  char* begin_string;            // fix header
  char* connection_type;         // connection type
  uint32_t heart_beat_interval;  // heart beat interval (seconds)
};

struct fix_default_settings defaults;

void GenerateCheckSum(char* buf, long bufLen) {
  unsigned sum = 0;
  long i;
  for (i = 0L; i < bufLen; i++) {
    unsigned val = (unsigned)buf[i];
    sum += val;
  }
  printf("CheckSum = %03d\n", (unsigned)(sum % 256));  // print result
}

void fix_loadconfig(char* ini_file) {
  (void)ini_file;

  char msg[] =
      "8=FIX.4.4\00135=A\00198=0\001108=30\001141=Y\001553=107948\001554="
      "B31iFgMs\001";
  printf("%s\n", msg);
  GenerateCheckSum(msg, sizeof(msg) / sizeof(msg[0]));
  exit(0);
}
