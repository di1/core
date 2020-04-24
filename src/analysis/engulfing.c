#include <analysis/engulfing.h>

#include "analysis/enumations.h"

enum RISKI_ERROR_CODE is_bullish_engulfing(size_t idx, struct chart* cht,
                                           enum DOUBLE_CANDLE_PATTERNS* res) {
  if (idx < 2) {
    *res = DOUBLE_CANDLE_PATTERNS_NONE;
    return RISKI_ERROR_CODE_NONE;
  }
  struct candle* cnd1 = NULL;
  TRACE(chart_get_candle(cht, idx, &cnd1));

  struct candle* cnd2 = NULL;
  TRACE(chart_get_candle(cht, idx-1, &cnd2));

  int64_t cnd1_o, cnd1_h, cnd1_l, cnd1_c = 0;
  TRACE(candle_open(cnd1, &cnd1_o));
  TRACE(candle_high(cnd1, &cnd1_h));
  TRACE(candle_low(cnd1, &cnd1_l));
  TRACE(candle_close(cnd1, &cnd1_c));

  // make sure this candle is going up
  if (cnd1_c <= cnd1_o) {
    *res = DOUBLE_CANDLE_PATTERNS_NONE;
    return RISKI_ERROR_CODE_NONE;
  }

  int64_t cnd2_o, cnd2_h, cnd2_l, cnd2_c = 0;
  TRACE(candle_open(cnd2, &cnd2_o));
  TRACE(candle_high(cnd2, &cnd2_h));
  TRACE(candle_low(cnd2, &cnd2_l));
  TRACE(candle_close(cnd2, &cnd2_c));

  // make sure this candle is going down
  if (cnd2_c >= cnd2_o) {
    *res = DOUBLE_CANDLE_PATTERNS_NONE;
    return RISKI_ERROR_CODE_NONE;
  }

  // compute body size
  size_t cnd1_body_size = cnd1_c - cnd1_o;
  size_t cnd2_body_size = cnd2_o - cnd2_c;

  // make sure the first body is longer than the second candle
  if (cnd1_body_size <= cnd2_body_size) {
    *res = DOUBLE_CANDLE_PATTERNS_NONE;
    return RISKI_ERROR_CODE_NONE;
  }

  // make sure the first candle "engulfes" the second candle
  if (cnd1_o < cnd2_c && cnd1_c > cnd1_o
      && cnd1_l < cnd2_l && cnd1_h > cnd2_h) {
    char* n = NULL;
    TRACE(chart_get_name(cht, &n));
    printf("[%s] bullish engulfing\n", n);
    *res = DOUBLE_CANDLE_PATTERNS_BULLISH_ENGULFING;
  } else {
    *res = DOUBLE_CANDLE_PATTERNS_NONE;
  }

  return RISKI_ERROR_CODE_NONE;
}
