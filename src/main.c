#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <iex/iex.h>
#include <log/log.h>
#include <book/book.h>
#include <chart/candle.h>
#include <chart/chart.h>
#include <security/security.h>
#include <exchange/exchange.h>
#include <server/server.h>
#include <analysis/analysis.h>

/**
 * Defines the command line arguments
 * that are possible with their values
 */
typedef struct {
  bool pcap_feed;
  char* pcap_feed_file;
} cli;

/**
 * Parses the comand line arguments
 */
cli* cli_parse(int argc, char** argv) {
  cli* options = (cli*) malloc(1*sizeof(cli));
  options->pcap_feed = false;
  options->pcap_feed_file = NULL;

  for (int i = 0; i < argc; ++i) {
    if (strcmp("-pcap_feed", argv[i]) == 0) {
      if (i+1 < argc) {
        options->pcap_feed = true;
        options->pcap_feed_file = argv[i+1];
      } else {
        log_error("%s", "-pcap_feed must be followed "
                        "by a file location");
        EXIT_FAILURE;
      }
    }
  }

  return options;
}

void valid_working_directory() {
  if (0 != access("./web/", F_OK)) {
    if (ENOENT == errno || ENOTDIR == errno) {
       // does not exist
       log_error("can not find directory ./web/");
       exit(1);
    }
  }
}

void usage(char* path) {
  printf("%s [-pcap_feed FILE][-pcap_list FILE1 FILE2 ...]\n", path);
  exit(1);
}

int main(int argc, char** argv) {

  if (argc == 1)
    usage(argv[0]);

  valid_working_directory();

  cli* options = cli_parse(argc, argv);

  pthread_t id;
  pthread_create(&id, NULL, server_start, NULL);

  analysis_init();

  if (options->pcap_feed) {
    iex_parse_deep(options->pcap_feed_file);
  }

  free(options);
  SERVER_INTERRUPTED = 1;

  pthread_join(id, NULL);
  return 0;
}
