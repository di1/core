#ifndef IEX_PACKET_
#define IEX_PACKET_

#include "types.h"

// the iex tp header
struct iex_tp_header {
  iex_byte_t version;
  iex_byte_t reserved;
  iex_short_t message_protocol_id;
  iex_int_t channel_id;
  iex_int_t session_id;
  iex_short_t payload_length;
  iex_short_t message_count;
  iex_long_t stream_offset;
  iex_long_t first_message_sequence_number;
  iex_timestamp_t send_time;
} __attribute__((packed));

// decifer the message block
struct iex_tp_message_block_header {
  iex_short_t message_length;
  iex_byte_t message_type;
} __attribute__((packed));

struct iex_system_event_message {
  iex_byte_t system_event;
  iex_timestamp_t ts;
} __attribute__((packed));

struct iex_security_directory_message {
  iex_byte_t flags;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_int_t round_lot_size;
  iex_price_t adjested_poc_price;
  iex_byte_t luld_tier;
} __attribute__((packed));

struct iex_trading_status_message {
  iex_byte_t trading_status;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_byte_t reason[4];
} __attribute__((packed));

struct iex_operational_halt_status_message {
  iex_byte_t operational_halt_status;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
} __attribute__((packed));

struct iex_short_sale_price_test_message {
  iex_byte_t short_sale_price_test_status;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_byte_t detail;
} __attribute__((packed));

struct iex_security_event_message {
  iex_byte_t security_event;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
} __attribute__((packed));

struct iex_price_level_update_message {
  iex_byte_t event_flags;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_int_t size;
  iex_price_t price;
} __attribute__((packed));

struct iex_trade_report_message {
  iex_byte_t sale_condition_flag;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_int_t size;
  iex_price_t price;
  iex_long_t trade_id;
} __attribute__((packed));

struct iex_official_price_message {
  iex_byte_t price_type;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_price_t official_price;
} __attribute__((packed));

struct iex_trade_break_message {
  iex_byte_t sale_condition_flags;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_int_t size;
  iex_price_t price;
  iex_long_t trade_id;
} __attribute__((packed));

struct iex_auction_information_message {
  iex_byte_t auction_type;
  iex_timestamp_t timestamp;
  iex_byte_t symbol[8];
  iex_int_t paired_shares;
  iex_price_t reference_price;
  iex_price_t indicative_clearing_price;
  iex_int_t imbalance_shares;
  iex_byte_t imbalance_side;
  iex_byte_t extension_number;
  iex_event_time_t scheduled_auction_time;
  iex_price_t auction_book_clearing_price;
  iex_price_t collar_reference_price;
  iex_price_t lower_auction_collar;
  iex_price_t upper_auction_collar;
} __attribute__((packed));

#endif
