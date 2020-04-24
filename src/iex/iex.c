#include <iex/iex.h>

#include "error_codes.h"

int IEX_SIGNAL_INTER = 0;
pcap_t* desc;

enum RISKI_ERROR_CODE iex_stop_parse() {
  pcap_breakloop(desc);
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Sets or appends a NULL character to the end of
 * a string that is not NULL terminated
 */
enum RISKI_ERROR_CODE symbol_sanitize(char* s, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (s[i] == ' ') {
      s[i] = '\x0';
      return RISKI_ERROR_CODE_NONE;
    }
  }
  TRACE(logger_error(RISKI_ERROR_CODE_INSUFFITIENT_SPACE, __func__,
                     __FILENAME__, __LINE__, "%.*s must have (nil) appened", n,
                     s));
  return RISKI_ERROR_CODE_UNKNOWN;
}

/**
 * A function pointer to a handler that will operate on the incoming
 * packets, the incoming packets from a file are indisquinshible
 * from an interface
 */
void packet_handler(unsigned char* userData, const struct pcap_pkthdr* pkthdr,
                    const unsigned char* packet);

/**
 * Processes the data inside the udp packet
 */
enum RISKI_ERROR_CODE iex_tp_handler(unsigned char* data);

/**
 * Prints the packet header for debug information
 */
enum RISKI_ERROR_CODE print_iex_tp_header(struct iex_tp_header* header) {
  PTR_CHECK(header, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  TRACE(
      logger_info(__func__, __FILENAME__, __LINE__, " === iex_tp_header ==="));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "version: 0x%x",
                    header->version));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "reserved: 0x%x",
                    header->reserved));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "message_protocol_id: %u",
                    header->message_protocol_id));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "channel_id: %u",
                    header->channel_id));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "session_id: %u",
                    header->session_id));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "payload_length: %u",
                    header->payload_length));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "message_count: %u",
                    header->message_count));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "stream_offset: %ld",
                    header->stream_offset));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "first_message_sequence_number: %ld",
                    header->first_message_sequence_number));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "send_time: %lu",
                    header->send_time));
  TRACE(
      logger_info(__func__, __FILENAME__, __LINE__, " === iex_tp_header ==="));
  return RISKI_ERROR_CODE_NONE;
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
enum RISKI_ERROR_CODE iex_parse_deep(char* file) {
  PTR_CHECK(file, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "parsing pcap file: %s",
                    file));

  // create a file descriptor for the pcap file
  char errbuff[PCAP_ERRBUF_SIZE];
  desc = pcap_open_offline(file, errbuff);

  // exit with error if troubles happened during
  // opening
  if (!desc) {
    TRACE(logger_error(RISKI_ERROR_CODE_UNKNOWN, __func__, __FILENAME__,
                       __LINE__, "%s", errbuff));
    exit(1);
  }

  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "creating exchange with name IEX"));
  TRACE(exchange_new("IEX", &iex_exchange));

  if (pcap_loop(desc, 0, packet_handler, NULL) < 0) {
    if (IEX_SIGNAL_INTER != 1) {
      TRACE(logger_info(__func__, __FILENAME__, __LINE__, "%s",
                        pcap_geterr(desc)));
      exit(1);
    }
  }

  // TODO do some sort of finalization to the data here?
  TRACE(exchange_free(&iex_exchange));
  pcap_close(desc);

  return RISKI_ERROR_CODE_NONE;
}

void packet_handler(unsigned char* userData, const struct pcap_pkthdr* pkthdr,
                    const unsigned char* packet) {
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
      unsigned char* data = (unsigned char*)(packet + sizeof(struct ether_header) +
                               sizeof(struct ip) + sizeof(struct udphdr));

      // offload the udp data processing out of this function
      TRACE_HAULT(iex_tp_handler(data));
    }
  }
}

/**
 * Parses a system event message, which tells us details about
 * if the market is open, after ours etc...
 */
enum RISKI_ERROR_CODE parse_system_event_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  switch (((struct iex_system_event_message*)payload)->system_event) {
    case START_OF_MESSAGES:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__, "start of messages"));
      break;
    case START_OF_SYSTEM_HOURS:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                        "start of system hours"));
      break;
    case START_OF_REGULAR_MARKET_HOURS:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                        "start of regular market hours"));
      break;
    case END_OF_REGULAR_MARKET_HOURS:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                        "end of regular market hours"));
      break;
    case END_OF_SYSTEM_HOURS:
      TRACE(
          logger_info(__func__, __FILENAME__, __LINE__, "end of system hours"));
      break;
    case END_OF_MESSAGES:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__, "end of messages"));
      break;
  }
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Tells us that there is something special for this
 * security
 */
enum RISKI_ERROR_CODE parse_security_directory_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct iex_security_directory_message* payload_data =
      (struct iex_security_directory_message*)(payload);

  TRACE(symbol_sanitize((char*)payload_data->symbol, 8));
  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "security directory message for %s", payload_data->symbol));

  // TODO do something with this information
  // TODO https://iextrading.com/docs/IEX%20DEEP%20Specification.pdf
  // TODO page 8
  //
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Tells us the current state of the security,
 * weather it is paused/haulted/released etc...
 */
enum RISKI_ERROR_CODE parse_trading_status_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  struct iex_trading_status_message* payload_data =
      (struct iex_trading_status_message*)(payload);
  (void)payload_data;

  /*
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "trading status message
  for %.8s", payload_data->symbol);
  */
  // TODO do something with this information
  //
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Indicates that the security has been halted
 */
enum RISKI_ERROR_CODE parse_operational_hault_status_message(void* payload) {
  struct iex_operational_halt_status_message* payload_data =
      (struct iex_operational_halt_status_message*)(payload);
  (void)payload_data;

  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "operation hault message for %.8s", payload_data->symbol));

  // TODO do something with this information
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE parse_short_sale_price_test_status_message(
    void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct iex_short_sale_price_test_message* payload_data =
      (struct iex_short_sale_price_test_message*)(payload);
  (void)payload_data;
  /*
  TRACE(logger_info(__func__, __FILENAME__, __LINE__, "short sale price test
  message for %.8s", payload_data->symbol);
  */

  // TODO do something with this information
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE parse_security_event_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  struct iex_security_event_message* payload_data =
      (struct iex_security_event_message*)(payload);

  TRACE(symbol_sanitize((char*)payload_data->symbol, 8));

  switch (payload_data->security_event) {
    case OPENING_PROCESS_COMPLETE:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                        "opening process complete %.8s", payload_data->symbol));
      break;
    case CLOSING_PROCESS_COMPLETE:
      TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                        "closing process complete %.8s", payload_data->symbol));
      break;
    default:
      TRACE(logger_error(RISKI_ERROR_CODE_UNKNOWN, __func__, __FILENAME__,
                         __LINE__,
                         "unknown security event message 0x%x symbol %.8s",
                         payload_data->security_event, payload_data->symbol));
      return RISKI_ERROR_CODE_INVALID_MESSAGE;
      exit(1);
  }

  // TODO might want to do more with this
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Tells us that a price update has happened to the order book.
 * The side was given in the message block and needs to be passed
 * though to this function.
 */
enum RISKI_ERROR_CODE parse_price_level_update_message(iex_byte_t side,
                                                       void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct iex_price_level_update_message* payload_data =
      (struct iex_price_level_update_message*)(payload);

  TRACE(symbol_sanitize((char*)payload_data->symbol, 8));
  struct security* cur_sec = NULL;

reget_security:
  TRACE(exchange_get(iex_exchange, (char*)payload_data->symbol, &cur_sec));

  if (cur_sec == NULL) {
    TRACE(exchange_put(iex_exchange, (char*)payload_data->symbol,
                       SECURITY_INTERVAL_MINUTE_NANOSECONDS));
    goto reget_security;
  }

  TRACE(security_book_update(cur_sec, side, payload_data->price,
                             payload_data->size));
  return RISKI_ERROR_CODE_NONE;
}

/**
 * The trade report message tells us when a trade has happened,
 * this will also be the latest price
 */
enum RISKI_ERROR_CODE parse_trade_report_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct iex_trade_report_message* payload_data =
      (struct iex_trade_report_message*)(payload);

  TRACE(symbol_sanitize((char*)payload_data->symbol, 8));
  struct security* cur_sec = NULL;

reget_security:
  TRACE(exchange_get(iex_exchange, (char*)payload_data->symbol, &cur_sec));

  if (cur_sec == NULL) {
    TRACE(exchange_put(iex_exchange, (char*)payload_data->symbol,
                       SECURITY_INTERVAL_MINUTE_NANOSECONDS));
    goto reget_security;
  }

  TRACE(security_chart_update(cur_sec, payload_data->price,
                              payload_data->timestamp));
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Tells us what the official opening and closing prices
 * Note that is only for stocks traded on IEX and will not
 * display non IEX opening prices
 */
enum RISKI_ERROR_CODE parse_official_price_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  struct iex_official_price_message* payload_data =
      (struct iex_official_price_message*)(payload);

  // TODO pass the information to the market about
  // the official opening and closing prices

  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "official price message for %.8s", payload_data->symbol));
  return RISKI_ERROR_CODE_NONE;
}

/**
 * Tells us that a security on IEX is broken
 * so it can no longer be traded that day
 */
enum RISKI_ERROR_CODE parse_trade_break_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  struct iex_trade_break_message* payload_data =
      (struct iex_trade_break_message*)(payload);

  // TODO this is IEX specific and is considered "rare"
  // TODO I'm unsure what to do with this

  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "trade break message %.8s", payload_data->symbol));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE parse_auction_information_message(void* payload) {
  PTR_CHECK(payload, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct iex_auction_information_message* payload_data =
      (struct iex_auction_information_message*)(payload);

  // TODO this is a beast to taccle and understand but
  // TODO is should not affect the order book or last
  // TODO traded prices and I think can be left alone for now

  TRACE(logger_info(__func__, __FILENAME__, __LINE__,
                    "auction information for %.8s", payload_data->symbol));

  return RISKI_ERROR_CODE_NONE;
}

/**
 * Parses the header data of the packet making sure it is
 * actually an iex packet and sending it of to a parse_*
 * function to do a task
 */
enum RISKI_ERROR_CODE iex_tp_handler(unsigned char* data) {
  PTR_CHECK(data, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // the header starts at position 0 of the data
  struct iex_tp_header* header = (struct iex_tp_header*)&data[0];

  if (!(header->message_protocol_id == 0x8004 && header->channel_id == 1)) {
    TRACE(logger_error(RISKI_ERROR_CODE_INVALID_PROTOCOL, __func__,
                       __FILENAME__, __LINE__, "unknown protocol\n"));
    return RISKI_ERROR_CODE_UNKNOWN;
  }

  if (header->payload_length == 0 && header->message_count == 0) {
    // this is a heartbeat message
    return RISKI_ERROR_CODE_NONE;
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
        TRACE(parse_system_event_message(payload_body));
        data = &data[sizeof(struct iex_system_event_message)];
        break;
      case SECURITY_DIRECTORY_MESSAGE:
        TRACE(parse_security_directory_message(payload_body));
        data = &data[sizeof(struct iex_security_directory_message)];
        break;
      case TRADING_STATUS_MESSAGE:
        TRACE(parse_trading_status_message(payload_body));
        data = &data[sizeof(struct iex_trading_status_message)];
        break;
      case OPERATIONAL_HAULT_STATUS_MESSAGE:
        TRACE(parse_operational_hault_status_message(payload_body));
        data = &data[sizeof(struct iex_operational_halt_status_message)];
        break;
      case SHORT_SALE_PRICE_TEST_STATUS_MESSAGE:
        TRACE(parse_short_sale_price_test_status_message(payload_body));
        data = &data[sizeof(struct iex_short_sale_price_test_message)];
        break;
      case SECURITY_EVENT_MESSAGE:
        TRACE(parse_security_event_message(payload_body));
        data = &data[sizeof(struct iex_security_event_message)];
        break;
      case PRICE_LEVEL_UPDATE_BUY_MESSAGE:
      case PRICE_LEVEL_UPDATE_SELL_MESSAGE:
        TRACE(parse_price_level_update_message(message_header->message_type,
                                               payload_body));
        data = &data[sizeof(struct iex_price_level_update_message)];
        break;
      case TRADE_REPORT_MESSAGE:
        TRACE(parse_trade_report_message(payload_body));
        data = &data[sizeof(struct iex_trade_report_message)];
        break;
      case OFFICIAL_PRICE_MESSAGE:
        TRACE(parse_official_price_message(payload_body));
        data = &data[sizeof(struct iex_official_price_message)];
        break;
      case TRADE_BREAK_MESSAGE:
        TRACE(parse_trade_break_message(payload_body));
        data = &data[sizeof(struct iex_trade_break_message)];
        break;
      case AUCTION_INFORMATION_MESSAGE:
        TRACE(parse_auction_information_message(payload_body));
        data = &data[sizeof(struct iex_auction_information_message)];
        break;
      default:
        print_iex_tp_header(header);
        return RISKI_ERROR_CODE_INVALID_MESSAGE;
    }
  }
  return RISKI_ERROR_CODE_NONE;
}
