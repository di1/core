#include <stdio.h>
#include <iex/iex.h>
#include <log/log.h>
#include <stdbool.h>
#include <string.h>

/**
 * Defines the command line arguments
 * that are possible with their values
 */
typedef struct {
  bool pcap_feed;
  char* pcap_feed_file;
} cli;

void agreement() {
  printf("riski (C) Vittorio Papandrea\n");
  printf("Usage of this program implies that the owner(s) of this project\n");
  printf("are not liable for any losses as the result of using riski as a\n");
  printf("tool for making decisions in any tradable market. This program \n");
  printf("is given free of warentee and liability.\n");
  printf("Please Trade Responsibilly\n");
  printf("\n\n");
}

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
        exit(1);
      }
    }
  }

  return options;
}

int main(int argc, char** argv) {
  agreement();

  cli* options = cli_parse(argc, argv);

  if (options->pcap_feed) {
    iex_parse_deep(options->pcap_feed_file);
  }
  
  free(options);
  return 0;
}
