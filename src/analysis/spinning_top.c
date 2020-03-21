#include <analysis/spinning_top.h>

#include "analysis/enumations.h"

enum RISKI_ERROR_CODE is_white_spinning_top(struct candle* cnd,
                                            enum SINGLE_CANDLE_PATTERNS* res) {
  PTR_CHECK(cnd, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *res = SINGLE_CANDLE_PATTERN_NONE;

  int64_t a = 0;
  TRACE(candle_high(cnd, &a));
  int64_t b = 0;
  TRACE(candle_low(cnd, &b));
  int64_t c = 0;
  TRACE(candle_close(cnd, &c));
  int64_t d = 0;
  TRACE(candle_open(cnd, &d));

  if (c > d && a != c && d != b && a - c == d - b && c - d == a - c) {
    *res = SINGLE_CANDLE_PATTERN_WHITE_SPINNING_TOP;
  }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE is_black_spinning_top(struct candle* cnd,
                                            enum SINGLE_CANDLE_PATTERNS* res) {
  PTR_CHECK(cnd, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *res = SINGLE_CANDLE_PATTERN_NONE;

  int64_t a = 0;
  TRACE(candle_high(cnd, &a));
  int64_t b = 0;
  TRACE(candle_low(cnd, &b));
  int64_t c = 0;
  TRACE(candle_close(cnd, &c));
  int64_t d = 0;
  TRACE(candle_open(cnd, &d));

  if (c > d && a != c && d != b && a - c == d - b && c - d == a - c) {
    *res = SINGLE_CANDLE_PATTERN_BLACK_SPINNING_TOP;
  }

  *res = SINGLE_CANDLE_PATTERN_NONE;

  return RISKI_ERROR_CODE_NONE;
}
