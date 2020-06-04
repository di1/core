#include <analysis/analysis.h>
#include <book/book.h>
#include <chart/candle.h>
#include <chart/chart.h>
#include <exchange/exchange.h>
#include <iex/iex.h>
#include <logger.h>
#include <oanda/oanda.h>
#include <pthread.h>
#include <security/search.h>
#include <security/security.h>
#include <server/server.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>
#include <unistd.h>

/**
 * Defines the command line arguments
 * that are possible with their values
 */
typedef struct {
  bool pcap_feed;
  bool oanda_feed;
  bool dev_web;
  bool compile;

  // 4 unused bytes here for padding
  char _p1[4];

  char *pcap_feed_file;
  char *fxpig_ini_file;
  char *oanda_key;
  char *locaion;

} cli;

/**
 * Parses the comand line arguments
 */
static cli *cli_parse(int argc, char **argv) {
  cli *options = (cli *)malloc(1 * sizeof(cli));
  options->pcap_feed = false;
  options->pcap_feed_file = NULL;
  options->fxpig_ini_file = NULL;
  options->dev_web = false;
  options->oanda_feed = false;
  options->oanda_key = NULL;
  options->compile = false;

  for (int i = 0; i < argc; ++i) {
    if (strcmp("-pcap_feed", argv[i]) == 0) {
      if (i + 1 < argc) {
        options->pcap_feed = true;
        options->pcap_feed_file = argv[i + 1];
      } else {
        printf("%s", "-pcap_feed must be followed "
                     "by a file location");
        exit(1);
      }
    } else if (strcmp("-oanda_feed", argv[i]) == 0) {
      if (i + 1 < argc) {
        options->oanda_feed = true;
        options->oanda_key = argv[i + 1];
      } else {
        printf("%s", "-oanda_feed must be followd by an api key\n");
      }
    } else if (strcmp("-dev-web", argv[i]) == 0) {
      options->dev_web = true;
    } else if (strcmp("-c", argv[i]) == 0) {
      if (i + 1 < argc) {
        options->compile = true;
        options->locaion = argv[i + 1];
      } else {
        printf("%s", "-compile feed must be followed by a file location\n");
      }
    }
  }

  return options;
}

static void valid_working_directory() {
  if (0 != access("./web/", F_OK)) {
    if (ENOENT == errno || ENOTDIR == errno) {
      // does not exist
      printf("can not find directory ./web/");
      exit(1);
    }
  }
}

static void __attribute__((noreturn)) usage(char *path) {
  printf("%s [-pcap_feed FILE][-fxpig FILE]\n", path);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc == 1)
    usage(argv[0]);

  valid_working_directory();

  cli *options = cli_parse(argc, argv);

  TRACE_HAULT(search_init("./symbols.csv"));

  pthread_t id;
  pthread_create(&id, NULL, server_start, NULL);

  if (!options->dev_web) {
    analysis_init();
    if (options->pcap_feed) {
      iex_parse_deep(options->pcap_feed_file);
    } else if (options->oanda_feed) {
      TRACE_HAULT(oanda_live(options->oanda_key));
    }
    analysis_cleanup();
    SERVER_INTERRUPTED = 1;
    pthread_join(id, NULL);
  }

  TRACE_HAULT(search_free());
  free(options);
  pthread_join(id, NULL);
  return 0;
}
