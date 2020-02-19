#ifndef IEX_TYPE_
#define IEX_TYPE_

#include <stdint.h>

#define IEX_PRIMARY "233.215.21.4"
#define IEX_SECONDARY "233.215.21.132"
#define IEX_TERTIARY "233.215.21.68"
#define IEX_TESTING "233.215.21.241"

#define IEX_POP_DISASTER_PORT 10378
#define IEX_TESTING_ITF_PORT 32001

// system events
#define SYSTEM_EVENT_MESSAGE 0x53
#define START_OF_MESSAGES 0x4f
#define START_OF_SYSTEM_HOURS 0x53
#define START_OF_REGULAR_MARKET_HOURS 0x52
#define END_OF_REGULAR_MARKET_HOURS 0x4d
#define END_OF_SYSTEM_HOURS 0x45
#define END_OF_MESSAGES 0x43

// security messages
#define SECURITY_DIRECTORY_MESSAGE 0x44
#define LULD_TIER_0 0x0
#define LULD_TIER_1 0x1
#define LULD_TIER_2 0x2

// trading status messages
#define TRADING_STATUS_MESSAGE 0x48
#define HALT_NEWS_PENDING "T1  "
#define IPO_NOT_YET_TRADING "IPO1"
#define IPO_DEFERRED "IPOD"
#define LEVEL_3_BREACHED "MCB3"
#define REASON_NOT_AVAILABLE "NA  "
#define HALT_NEWS_DISSEMINATION "T2  "
#define IPO_ORDER_ACCEPTANCE_PERIOD "IPO2"
#define IPO_PRELAUNCH_PERIOD "IPO3"
#define LEVEL_1_BREACHED "MCB1"
#define LEVEL_2_BREACHED "MCB2"

// operational halt status
#define OPERATIONAL_HAULT_STATUS_MESSAGE 0x4f
#define IEX_SPECIFIC_OPERATIONAL_TRADING_HALT 0x4f
#define NOT_OPERATIONALLY_HALTED_ON_IEX 0x4e

// short sale price test status message
#define SHORT_SALE_PRICE_TEST_STATUS_MESSAGE 0x50
#define SHORT_SALE_PRICE_TEST_NOT_IN_EFFECT 0x0
#define SHORT_SALE_PRICE_TEST_IN_EFFECT 0x1
#define SHORT_SALE_PRICE_TEST_ACTIVATED 0x41
#define SHORT_SALE_PRICE_TEST_CONTINUED 0x43
#define SHORT_SALE_PRICE_TEST_DEACTIVATED 0x44
#define SHORT_SALE_PRICE_TEST_NO_DETAIL 0x4e

// security event message
#define SECURITY_EVENT_MESSAGE 0x45
#define OPENING_PROCESS_COMPLETE 0x4f
#define CLOSING_PROCESS_COMPLETE 0x43

// price level update message
#define PRICE_LEVEL_UPDATE_BUY_MESSAGE 0x38
#define PRICE_LEVEL_UPDATE_SELL_MESSAGE 0x35
#define ORDER_BOOK_IN_TRANSITION 0x0
#define ORDER_BOOK_TRANSITION_COMPLETE 0x1

// trade report message
#define TRADE_REPORT_MESSAGE 0X54

// official price message
#define OFFICIAL_PRICE_MESSAGE 0x58
#define OFFICIAL_OPENING_PRICE 0x51
#define OFFICIAL_CLOSING_PRICE 0x4d

// trade break message
#define TRADE_BREAK_MESSAGE 0x42

// auction information message
#define AUCTION_INFORMATION_MESSAGE 0x41
#define OPENING_ACUTION 0x4f
#define CLOSING_AUCTION 0x43
#define IPO_AUCTION 0x49
#define HALT_AUCTION 0x48
#define VOLATILITY_AUCTION 0x56
#define BUY_SIDE_IMBALANCE 0x42
#define SELL_SIDE_IMBALANCE 0x53
#define NO_IMBALANCE 0x4e

typedef int64_t iex_long_t;
typedef uint32_t iex_int_t;
typedef uint16_t iex_short_t;
typedef unsigned char iex_byte_t;
typedef uint64_t iex_timestamp_t;
typedef int64_t iex_price_t;
typedef uint32_t iex_event_time_t;

#endif
