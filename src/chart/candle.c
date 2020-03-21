#include <chart/candle.h>

#include "error_codes.h"

/*
 * Represents a candle
 * @param {int64_t} open The open price
 * @param {int64_t} high The high price
 * @param {int64_t} low The low price
 * @param {int64_t} close The close price
 * @param {uint64_t} start_time The start_time price
 * @param {uint64_t} end_time The end_time price
 * @param {uint64_t} volume The volume of the candle
 */
struct candle {
  int64_t open;
  int64_t high;
  int64_t low;
  int64_t close;
  uint64_t start_time;
  uint64_t end_time;
  uint64_t volume;
};

enum RISKI_ERROR_CODE candle_new(int64_t price, uint64_t time,
                                 struct candle** cnd) {
  struct candle* c = (struct candle*)malloc(1 * sizeof(struct candle));
  PTR_CHECK(c, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  c->open = price;
  c->high = price;
  c->low = price;
  c->close = price;
  c->start_time = time;
  c->end_time = time;
  c->volume = 0;

  *cnd = c;
  return RISKI_ERROR_CODE_NONE;
}

#define CREATE_CANDLE_GET_FUNCTION(NAME, TYPE, ELEMENT)            \
  enum RISKI_ERROR_CODE NAME(struct candle* c, TYPE* t) {          \
    PTR_CHECK(c, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT); \
    *t = c->ELEMENT;                                               \
    return RISKI_ERROR_CODE_NONE;                                  \
  }
CREATE_CANDLE_GET_FUNCTION(candle_volume, uint64_t, volume)
CREATE_CANDLE_GET_FUNCTION(candle_open, int64_t, open)
CREATE_CANDLE_GET_FUNCTION(candle_high, int64_t, high)
CREATE_CANDLE_GET_FUNCTION(candle_low, int64_t, low)
CREATE_CANDLE_GET_FUNCTION(candle_close, int64_t, close)
CREATE_CANDLE_GET_FUNCTION(candle_start, uint64_t, start_time)
CREATE_CANDLE_GET_FUNCTION(candle_end, uint64_t, end_time)
#undef CREATE_CANDLE_GET_FUNCTION

enum RISKI_ERROR_CODE candle_update(struct candle* c, int64_t price,
                                    uint64_t time) {
  PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  c->volume += 1;
  // set the close time only if the last price time
  // is greater than the most recent price
  if (time > c->end_time) {
    c->end_time = time;
    c->close = price;
  }

  // update the high and low
  // the time doesn't matter in this case
  if (price > c->high) c->high = price;
  if (price < c->low) c->low = price;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE candle_free(struct candle** c) {
  PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(*c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  free(*c);
  *c = NULL;
  return RISKI_ERROR_CODE_NONE;
}

// convert a candle struct to json
enum RISKI_ERROR_CODE candle_json(struct candle* c, char** json) {
  /**
   * {
   *  "candle" : {
   *    "o": 00000000000000000000,
   *    "h": 00000000000000000000,
   *    "l": 00000000000000000000,
   *    "c": 00000000000000000000,
   *    "s": 00000000000000000000,
   *    "e": 00000000000000000000,
   *    "v": 00000000000000000000
   *  }
   * }
   */

  // max 170 characters (no whitespace or new lines
  char* buf = (char*)malloc(JSON_CANDLE_MAX_LEN * sizeof(char));
  PTR_CHECK(buf, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  int ret = sprintf(buf,
                    "{\"candle\":{"
                    "\"o\":%lld,\"h\":%lld,\"l\":%lld,\"c\":%lld,\"s\":%llu,"
                    "\"e\":%llu,\"v\":%llu}}",
                    (long long)c->open, (long long)c->high, (long long)c->low,
                    (long long)c->close, (unsigned long long)c->start_time,
                    (unsigned long long)c->end_time, (long long)c->volume);

  if (ret == -1) {
    return RISKI_ERROR_CODE_JSON_CREATION;
  }

  *json = buf;
  return RISKI_ERROR_CODE_NONE;
}
