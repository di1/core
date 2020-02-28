#include <analysis/analysis.h>
#include <book/book.h>
#include <chart/candle.h>
#include <chart/chart.h>
#include <exchange/exchange.h>
#include <fxpig/fxpig.h>
#include <fxpig/ini.h>
#include <iex/iex.h>
#include <log/log.h>
#include <pthread.h>
#include <security/security.h>
#include <server/server.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Defines the command line arguments
 * that are possible with their values
 */
typedef struct {
  bool pcap_feed;
  char* pcap_feed_file;
  bool fxpig;
  char* fxpig_ini_file;
} cli;

/**
 * Parses the comand line arguments
 */
cli* cli_parse(int argc, char** argv) {
  cli* options = (cli*)malloc(1 * sizeof(cli));
  options->pcap_feed = false;
  options->pcap_feed_file = NULL;
  options->fxpig = false;
  options->fxpig_ini_file = NULL;

  for (int i = 0; i < argc; ++i) {
    if (strcmp("-pcap_feed", argv[i]) == 0) {
      if (i + 1 < argc) {
        options->pcap_feed = true;
        options->pcap_feed_file = argv[i + 1];
      } else {
        log_error("%s",
                  "-pcap_feed must be followed "
                  "by a file location");
        exit(1);
      }
    } else if (strcmp("-fxpig", argv[i]) == 0) {
      if (i + 1 < argc) {
        options->fxpig = true;
        options->fxpig_ini_file = argv[i + 1];
      } else {
        log_error("%s", "-fxpig must  be followed by a .ini file");
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
  if (argc == 1) usage(argv[0]);

  valid_working_directory();

  cli* options = cli_parse(argc, argv);

  pthread_t id;
  pthread_create(&id, NULL, server_start, NULL);

  if (options->pcap_feed) {
    analysis_init();
    iex_parse_deep(options->pcap_feed_file);
  } else if (options->fxpig) {
    struct fxpig_ini_config* cfg = fxpig_ini_parse(options->fxpig_ini_file);
    fxpig_live(cfg);
    fxpig_ini_free(&cfg);
  }

  free(options);
  analysis_cleanup();
  SERVER_INTERRUPTED = 1;
  pthread_join(id, NULL);
  return 0;
}
