#include <iex/iex.h>

#define TIME(CODE, MESSAGE)                                     \
  do {                                                          \
    clock_t begin = clock();                                    \
    CODE clock_t end = clock();                                 \
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC; \
    log_trace("%s %lf", MESSAGE, time_spent);                   \
  } while (0);

int IEX_SIGNAL_INTER = 0;
pcap_t* desc;

void iex_stop_parse() { pcap_breakloop(desc); }

/**
 * Sets or appends a NULL character to the end of
 * a string that is not NULL terminated
 */
void symbol_sanitize(char* s, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (s[i] == ' ') {
      s[i] = '\x0';
      return;
    }
  }
  log_error("%.*s must have (nil) appened", n, s);
}

/**
 * A function pointer to a handler that will operate on the incoming
 * packets, the incoming packets from a file are indisquinshible
 * from an interface
 */
void packet_handler(u_char* userData, const struct pcap_pkthdr* pkthdr,
                    const u_char* packet);

/**
 * Processes the data inside the udp packet
 */
void iex_tp_handler(u_char* data);

/**
 * Prints the packet header for debug information
 */
void print_iex_tp_header(struct iex_tp_header* header) {
  log_debug(" === iex_tp_header ===");
  log_debug("version: 0x%x", header->version);
  log_debug("reserved: 0x%x", header->reserved);
  log_debug("message_protocol_id: %u", header->message_protocol_id);
  log_debug("channel_id: %u", header->channel_id);
  log_debug("session_id: %u", header->session_id);
  log_debug("payload_length: %u", header->payload_length);
  log_debug("message_count: %u", header->message_count);
  log_debug("stream_offset: %ld", header->stream_offset);
  log_debug("first_message_sequence_number: %ld",
            header->first_message_sequence_number);
  log_debug("send_time: %lu", header->send_time);
  log_debug(" === iex_tp_header ===");
}

bool is_iex_traffic(char* ip_src, char* ip_dst) {
  return strcmp(ip_src, IEX_PRIMARY) == 0 ||
         strcmp(ip_src, IEX_SECONDARY) == 0 ||
         strcmp(ip_src, IEX_TERTIARY) == 0 ||
         strcmp(ip_dst, IEX_PRIMARY) == 0 ||
         strcmp(ip_dst, IEX_SECONDARY) == 0 ||
         strcmp(ip_dst, IEX_TERTIARY) == 0;
}

/**
 * Entry point to parsing an iex historical deep pcap file
 */
void iex_parse_deep(char* file) {
  log_trace("given pcap file: %s", file);
  log_trace("offline iex deep");

  // create a file descriptor for the pcap file
  char errbuff[PCAP_ERRBUF_SIZE];
  desc = pcap_open_offline(file, errbuff);

  // exit with error if troubles happened during
  // opening
  if (!desc) {
    log_error("%s", errbuff);
    exit(1);
  }

  log_trace("creating exchange with name IEX");
  iex_exchange = exchange_new("IEX");

  if (pcap_loop(desc, 0, packet_handler, NULL) < 0) {
    if (IEX_SIGNAL_INTER != 1) {
      log_error("%s", pcap_geterr(desc));
      exit(1);
    }
  }

  // TODO do some sort of finalization to the data here?
  exchange_free(&iex_exchange);
  pcap_close(desc);
}

void packet_handler(u_char* userData, const struct pcap_pkthdr* pkthdr,
                    const u_char* packet) {
  // we won't be passing any user defined information
  (void)userData;
  (void)pkthdr;

  // we only care about ethernet traffic so validate this packet
  // is an ethernet packet
  const struct ether_header* ethernet_header;
  ethernet_header = (struct ether_header*)packet;

  // validate type ethernet packet
  if (ntohs(ethernet_header->ether_type) == ETHERTYPE_IP) {
    // iex traffic comes from tcp traffic so make sure this is
    // a udp and not tcp traffic, also it comes from a set IP
    // we will filter out all the other traffic
    const struct ip* ip_header;
    ip_header = (struct ip*)(packet + sizeof(struct ether_header));

    // convert src and dst ip addresses into text form
    char ip_src[INET_ADDRSTRLEN];
    char ip_dst[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip_header->ip_src), ip_src, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_header->ip_dst), ip_dst, INET_ADDRSTRLEN);

    // verify udp packet and verify src
    if (ip_header->ip_p == IPPROTO_UDP && is_iex_traffic(ip_src, ip_dst)) {
      // extract the packet data
      u_char* data = (u_char*)(packet + sizeof(struct ether_header) +
                               sizeof(struct ip) + sizeof(struct udphdr));

      // offload the udp data processing out of this function
      iex_tp_handler(data);
    }
  }
}

/**
 * Parses a system event message, which tells us details about
 * if the market is open, after ours etc...
 */
void parse_system_event_message(void* payload) {
  switch (((struct iex_system_event_message*)payload)->system_event) {
    case START_OF_MESSAGES:
      log_trace("start of messages");
      break;
    case START_OF_SYSTEM_HOURS:
      log_trace("start of system hours");
      break;
    case START_OF_REGULAR_MARKET_HOURS:
      log_trace("start of regular market hours");
      break;
    case END_OF_REGULAR_MARKET_HOURS:
      log_trace("end of regular market hours");
      break;
    case END_OF_SYSTEM_HOURS:
      log_trace("end of system hours");
      break;
    case END_OF_MESSAGES:
      log_trace("end of messages");
      break;
  }
}

/**
 * Tells us that there is something special for this
 * security
 */
void parse_security_directory_message(void* payload) {
  struct iex_security_directory_message* payload_data =
      (struct iex_security_directory_message*)(payload);

  symbol_sanitize((char*)payload_data->symbol, 8);
  log_trace("security directory message for %s", payload_data->symbol);

  // TODO do something with this information
  // TODO https://iextrading.com/docs/IEX%20DEEP%20Specification.pdf
  // TODO page 8
}

/**
 * Tells us the current state of the security,
 * weather it is paused/haulted/released etc...
 */
void parse_trading_status_message(void* payload) {
  struct iex_trading_status_message* payload_data =
      (struct iex_trading_status_message*)(payload);
  (void)payload_data;

  /*
  log_trace("trading status message for %.8s", payload_data->symbol);
  */
  // TODO do something with this information
}

/**
 * Indicates that the security has been halted
 */
void parse_operational_hault_status_message(void* payload) {
  struct iex_operational_halt_status_message* payload_data =
      (struct iex_operational_halt_status_message*)(payload);
  (void)payload_data;
  /*
  log_trace("operation hault message for %.8s", payload_data->symbol);
  */
  // TODO do something with this information
}

void parse_short_sale_price_test_status_message(void* payload) {
  struct iex_short_sale_price_test_message* payload_data =
      (struct iex_short_sale_price_test_message*)(payload);
  (void)payload_data;
  /*
  log_trace("short sale price test message for %.8s", payload_data->symbol);
  */

  // TODO do something with this information
}

void parse_security_event_message(void* payload) {
  struct iex_security_event_message* payload_data =
      (struct iex_security_event_message*)(payload);

  symbol_sanitize((char*)payload_data->symbol, 8);

  switch (payload_data->security_event) {
    case OPENING_PROCESS_COMPLETE:
      /*
      log_trace("opening process complete %.8s",
          payload_data->symbol);
      */
      break;
    case CLOSING_PROCESS_COMPLETE:
      /*
      log_trace("closing process complete %.8s",
          payload_data->symbol);
      */
      break;
    default:
      log_error("unknown security event message 0x%x symbol %.8s",
                payload_data->security_event, payload_data->symbol);
      exit(1);
  }

  // TODO might want to do more with this
}

/**
 * Tells us that a price update has happened to the order book.
 * The side was given in the message block and needs to be passed
 * though to this function.
 */
void parse_price_level_update_message(iex_byte_t side, void* payload) {
  struct iex_price_level_update_message* payload_data =
      (struct iex_price_level_update_message*)(payload);

  symbol_sanitize((char*)payload_data->symbol, 8);
  struct security* cur_sec = NULL;

reget_security:
  cur_sec = exchange_get(iex_exchange, (char*)payload_data->symbol);

  if (cur_sec == NULL) {
    exchange_put(iex_exchange, (char*)payload_data->symbol,
                 SECURITY_INTERVAL_MINUTE_NANOSECONDS);
    goto reget_security;
  }

  security_book_update(cur_sec, side, payload_data->price, payload_data->size);
}

/**
 * The trade report message tells us when a trade has happened,
 * this will also be the latest price
 */
void parse_trade_report_message(void* payload) {
  struct iex_trade_report_message* payload_data =
      (struct iex_trade_report_message*)(payload);

  symbol_sanitize((char*)payload_data->symbol, 8);
  struct security* cur_sec = NULL;

reget_security:
  cur_sec = exchange_get(iex_exchange, (char*)payload_data->symbol);

  if (cur_sec == NULL) {
    exchange_put(iex_exchange, (char*)payload_data->symbol,
                 SECURITY_INTERVAL_MINUTE_NANOSECONDS);
    goto reget_security;
  }

  security_chart_update(cur_sec, payload_data->price, payload_data->timestamp);
}

/**
 * Tells us what the official opening and closing prices
 * Note that is only for stocks traded on IEX and will not
 * display non IEX opening prices
 */
void parse_official_price_message(void* payload) {
  struct iex_official_price_message* payload_data =
      (struct iex_official_price_message*)(payload);

  // TODO pass the information to the market about
  // the official opening and closing prices

  log_trace("official price message for %.8s", payload_data->symbol);
}

/**
 * Tells us that a security on IEX is broken
 * so it can no longer be traded that day
 */
void parse_trade_break_message(void* payload) {
  struct iex_trade_break_message* payload_data =
      (struct iex_trade_break_message*)(payload);

  // TODO this is IEX specific and is considered "rare"
  // TODO I'm unsure what to do with this

  log_trace("trade break message %.8s", payload_data->symbol);
}

void parse_auction_information_message(void* payload) {
  struct iex_auction_information_message* payload_data =
      (struct iex_auction_information_message*)(payload);

  // TODO this is a beast to taccle and understand but
  // TODO is should not affect the order book or last
  // TODO traded prices and I think can be left alone for now

  // log_trace("auction information for %.8s", payload_data->symbol);
  (void)payload_data;
}

/**
 * Parses the header data of the packet making sure it is
 * actually an iex packet and sending it of to a parse_*
 * function to do a task
 */
void iex_tp_handler(u_char* data) {
  // the header starts at position 0 of the data
  struct iex_tp_header* header = (struct iex_tp_header*)&data[0];

  if (!(header->message_protocol_id == 0x8004 && header->channel_id == 1)) {
    log_trace("unknown protocol\n");
    return;
  }

  if (header->payload_length == 0 && header->message_count == 0) {
    // this is a heartbeat message
    return;
  }

  data = &data[sizeof(struct iex_tp_header)];

  // to do bug, this only reads the first message of the message block
  // add loop to read all messages

  for (iex_short_t i = 0; i < header->message_count; ++i) {
    // read the message block to figure out what kind of message this is
    struct iex_tp_message_block_header* message_header =
        (struct iex_tp_message_block_header*)&data[0];

    data = &data[sizeof(struct iex_tp_message_block_header)];

    void* payload_body = &data[0];

    // switch through the different message types
    switch (message_header->message_type) {
      // administrative messages to tell us where in the trading day
      // we are
      case SYSTEM_EVENT_MESSAGE:
        parse_system_event_message(payload_body);
        data = &data[sizeof(struct iex_system_event_message)];
        break;
      case SECURITY_DIRECTORY_MESSAGE:
        parse_security_directory_message(payload_body);
        data = &data[sizeof(struct iex_security_directory_message)];
        break;
      case TRADING_STATUS_MESSAGE:
        parse_trading_status_message(payload_body);
        data = &data[sizeof(struct iex_trading_status_message)];
        break;
      case OPERATIONAL_HAULT_STATUS_MESSAGE:
        parse_operational_hault_status_message(payload_body);
        data = &data[sizeof(struct iex_operational_halt_status_message)];
        break;
      case SHORT_SALE_PRICE_TEST_STATUS_MESSAGE:
        parse_short_sale_price_test_status_message(payload_body);
        data = &data[sizeof(struct iex_short_sale_price_test_message)];
        break;
      case SECURITY_EVENT_MESSAGE:
        parse_security_event_message(payload_body);
        data = &data[sizeof(struct iex_security_event_message)];
        break;
      case PRICE_LEVEL_UPDATE_BUY_MESSAGE:
      case PRICE_LEVEL_UPDATE_SELL_MESSAGE:
        parse_price_level_update_message(message_header->message_type,
                                         payload_body);
        data = &data[sizeof(struct iex_price_level_update_message)];
        break;
      case TRADE_REPORT_MESSAGE:
        parse_trade_report_message(payload_body);
        data = &data[sizeof(struct iex_trade_report_message)];
        break;
      case OFFICIAL_PRICE_MESSAGE:
        parse_official_price_message(payload_body);
        data = &data[sizeof(struct iex_official_price_message)];
        break;
      case TRADE_BREAK_MESSAGE:
        parse_trade_break_message(payload_body);
        data = &data[sizeof(struct iex_trade_break_message)];
        break;
      case AUCTION_INFORMATION_MESSAGE:
        parse_auction_information_message(payload_body);
        data = &data[sizeof(struct iex_auction_information_message)];
        break;
      default:
        print_iex_tp_header(header);
        exit(1);
        break;
    }
  }
}
