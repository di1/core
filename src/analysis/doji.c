#include <analysis/doji.h>

/*
 * String representation of the error codes
 */
const char* DOJI_ERROR_CODE_STR[1] = {"DOJI_NO_ERROR"};

enum DOJI_ERROR_CODE perform_doji_dragonfly(struct chart* cht,
                                            size_t end_candle) {
  struct candle* last_candle = NULL;
  TRACE(chart_get_candle(cht, end_candle - 1, &last_candle));

  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == c && o == h && l < o) {
    return SINGLE_CANDLE_PATTERN_DOJI_DRAGONFLY;
  }
  return SINGLE_CANDLE_PATTERN_NONE;
}

enum DOJI_ERROR_CODE perform_doji_gravestone(struct chart* cht,
                                             size_t end_candle) {
  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == c && o == l && h > o) {
    return SINGLE_CANDLE_PATTERN_DOJI_GRAVESTONE;
  }
  return SINGLE_CANDLE_PATTERN_NONE;
}

enum DOJI_ERROR_CODE is_doji_generic(struct chart* cht, size_t end_candle) {
  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == c && h != l) {
    return SINGLE_CANDLE_PATTERN_DOJI_GENERIC;
  }
  return SINGLE_CANDLE_PATTERN_NONE;
}
