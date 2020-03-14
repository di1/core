#include <analysis/spinning_top.h>

enum SINGLE_CANDLE_PATTERNS is_white_spinning_top(struct candle* cnd) {
  int64_t a = candle_high(cnd);
  int64_t b = candle_low(cnd);
  int64_t c = candle_close(cnd);
  int64_t d = candle_open(cnd);

  if (c > d && a != c && d != b && a - c == d - b && c - d == a - c)
    return SINGLE_CANDLE_PATTERN_WHITE_SPINNING_TOP;
  return SINGLE_CANDLE_PATTERN_NONE;
}

enum SINGLE_CANDLE_PATTERNS is_black_spinning_top(struct candle* cnd) {
  int64_t a = candle_high(cnd);
  int64_t b = candle_low(cnd);
  int64_t c = candle_open(cnd);
  int64_t d = candle_close(cnd);

  if (c > d && a != c && d != b && a - c == d - b && c - d == a - c)
    return SINGLE_CANDLE_PATTERN_BLACK_SPINNING_TOP;
  return SINGLE_CANDLE_PATTERN_NONE;
}
