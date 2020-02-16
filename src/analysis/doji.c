#include <analysis/doji.h>

enum SINGLE_CANDLE_PATTERNS is_doji_top(struct candle* cnd) {
  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == c && o == h && l < o)
    return SINGLE_CANDLE_PATTERN_DOJI_TOP;
  return SINGLE_CANDLE_PATTERN_NONE;
}


enum SINGLE_CANDLE_PATTERNS is_doji_bot(struct candle* cnd) {
  int64_t o = candle_open(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);
  int64_t c = candle_close(cnd);

  if (o == c && o == l && h > o)
    return SINGLE_CANDLE_PATTERN_DOJI_BOT;
  return SINGLE_CANDLE_PATTERN_NONE;
}

enum SINGLE_CANDLE_PATTERNS is_doji_generic(struct candle* cnd) {
  int64_t o = candle_open(cnd);
  int64_t c = candle_close(cnd);
  int64_t h = candle_high(cnd);
  int64_t l = candle_low(cnd);

  if (o == c && h != l)
    return SINGLE_CANDLE_PATTERN_DOJI_GENERIC;
  return SINGLE_CANDLE_PATTERN_NONE;
}
