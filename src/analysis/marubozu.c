#include <analysis/marubozu.h>

enum SINGLE_CANDLE_PATTERNS is_white_marubozu(struct candle* cnd) {
  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == l && c == h && l != h) return SINGLE_CANDLE_PATTERN_WHITE_MARUBOZU;
  return SINGLE_CANDLE_PATTERN_NONE;
}

enum SINGLE_CANDLE_PATTERNS is_black_marubozu(struct candle* cnd) {
  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == h && c == l && h != l) return SINGLE_CANDLE_PATTERN_BLACK_MARUBOZU;
  return SINGLE_CANDLE_PATTERN_NONE;
}
