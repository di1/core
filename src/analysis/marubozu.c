#include "chart/chart.h"
#include <analysis/marubozu.h>

enum SINGLE_CANDLE_PATTERNS is_white_marubozu(struct candle* c) {
  if (candle_open(c) == candle_low(c) &&
      candle_close(c) == candle_high(c))
    return SINGLE_CANDLE_PATTERN_WHITE_MARUBOZU;
  return SINGLE_CANDLE_PATTERN_NONE;
}

enum SINGLE_CANDLE_PATTERNS is_black_marubozu(struct candle* c) {
  if (candle_open(c) == candle_high(c) &&
      candle_close(c) == candle_low(c))
    return SINGLE_CANDLE_PATTERN_BLACK_MARUBOZU;
  return SINGLE_CANDLE_PATTERN_NONE;
}
