#include <chart/candle.h>

#define MAX_INT_STR_LEN ((CHAR_BIT * sizeof(int) - 1) / 3 + 2)

/*
 * Represents a candle
 * @param {int64_t} open The open price
 * @param {int64_t} high The high price
 * @param {int64_t} low The low price
 * @param {int64_t} close The close price
 * @param {int64_t} best_bid The best bid price
 * @param {int64_t} best_ask The best ask price
 * @param {uint64_t} start_time The start_time price
 * @param {uint64_t} end_time The end_time price
 * @param {uint64_t} volume The volume of the candle
 */
struct candle {
  int64_t open;
  int64_t high;
  int64_t low;
  int64_t close;
  int64_t best_bid;
  int64_t best_ask;
  uint64_t start_time;
  uint64_t end_time;
  uint64_t volume;
};

enum RISKI_ERROR_CODE candle_new(int64_t price, uint64_t time,
                                 struct candle **cnd) {
  struct candle *c = (struct candle *)malloc(1 * sizeof(struct candle));
  PTR_CHECK(c, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  c->open = price;
  c->high = price;
  c->low = price;
  c->close = price;
  c->start_time = time;
  c->end_time = time;
  c->best_ask = price;
  c->best_bid = price;
  c->volume = 0;
  *cnd = c;

  return RISKI_ERROR_CODE_NONE;
}

#define CREATE_CANDLE_GET_FUNCTION(NAME, TYPE, ELEMENT)                        \
  enum RISKI_ERROR_CODE NAME(struct candle *c, TYPE *t) {                      \
    PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);             \
    *t = c->ELEMENT;                                                           \
    return RISKI_ERROR_CODE_NONE;                                              \
  }

CREATE_CANDLE_GET_FUNCTION(candle_volume, uint64_t, volume)
CREATE_CANDLE_GET_FUNCTION(candle_open, int64_t, open)
CREATE_CANDLE_GET_FUNCTION(candle_high, int64_t, high)
CREATE_CANDLE_GET_FUNCTION(candle_low, int64_t, low)
CREATE_CANDLE_GET_FUNCTION(candle_close, int64_t, close)
CREATE_CANDLE_GET_FUNCTION(candle_start, uint64_t, start_time)
CREATE_CANDLE_GET_FUNCTION(candle_end, uint64_t, end_time)

#undef CREATE_CANDLE_GET_FUNCTION

enum RISKI_ERROR_CODE candle_update(struct candle *c, int64_t price,
                                    uint64_t time) {
  PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // set the close time only if the last price time
  // is greater than the most recent price
  if (time >= c->end_time) {
    c->volume += 1;
    c->end_time = time;
    c->close = price;
  }

  // update the high and low
  // the time doesn't matter in this case
  if (price > c->high)
    c->high = price;
  if (price < c->low)
    c->low = price;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE candle_free(struct candle **c) {
  PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(*c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  free(*c);
  *c = NULL;
  return RISKI_ERROR_CODE_NONE;
}

// convert a candle struct to json
enum RISKI_ERROR_CODE candle_json(struct candle *c, char **json) {
  /**
   * {
   *  "candle" : {
   *    "o": 00000000000000000000,
   *    "h": 00000000000000000000,
   *    "l": 00000000000000000000,
   *    "c": 00000000000000000000,
   *    "s": 00000000000000000000,
   *    "e": 00000000000000000000,
   *    "v": 00000000000000000000,
   *    "b": 00000000000000000000,
   *    "a": 00000000000000000000
   *  }
   * }
   */

  char type_str[MAX_INT_STR_LEN];

  // max 170 characters (no white space or new lines)
  struct string_builder *sb = NULL;
  TRACE(string_builder_new(&sb));

  TRACE(string_builder_append(sb, "{\"candle\":{"));

  TRACE(string_builder_append(sb, "\"o\":"));
  sprintf(type_str, "%d", (int)c->open);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"h\":"));
  sprintf(type_str, "%d", (int)c->high);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"l\":"));
  sprintf(type_str, "%d", (int)c->low);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"c\":"));
  sprintf(type_str, "%d", (int)c->close);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"s\":"));
  sprintf(type_str, "%d", (int)c->start_time);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"e\":"));
  sprintf(type_str, "%d", (int)c->end_time);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"v\":"));
  sprintf(type_str, "%d", (int)c->volume);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"b\":"));
  sprintf(type_str, "%d", (int)c->best_bid);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, ",\"a\":"));
  sprintf(type_str, "%d", (int)c->best_ask);
  TRACE(string_builder_append(sb, type_str));

  TRACE(string_builder_append(sb, "}}"));

  char *ret = NULL;
  TRACE(string_builder_str(sb, &ret));

  TRACE(string_builder_free(&sb));

  *json = ret;
  return RISKI_ERROR_CODE_NONE;
}

#undef MAX_INT_STR_LEN
