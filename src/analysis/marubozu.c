#include <analysis/marubozu.h>

#include "analysis/enumations.h"

enum RISKI_ERROR_CODE is_white_marubozu(struct candle* cnd,
                                        enum SINGLE_CANDLE_PATTERNS* res) {
  PTR_CHECK(cnd, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  *res = SINGLE_CANDLE_PATTERN_NONE;

  int64_t o = 0;
  TRACE(candle_open(cnd, &o));
  int64_t h = 0;
  TRACE(candle_high(cnd, &h));
  int64_t l = 0;
  TRACE(candle_low(cnd, &l));
  int64_t c = 0;
  TRACE(candle_close(cnd, &c));

  if (o == l && c == h && l != h) {
    *res = SINGLE_CANDLE_PATTERN_WHITE_MARUBOZU;
  }

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE is_black_marubozu(struct candle* cnd,
                                        enum SINGLE_CANDLE_PATTERNS* res) {
  PTR_CHECK(cnd, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *res = SINGLE_CANDLE_PATTERN_NONE;

  int64_t o = 0;
  TRACE(candle_open(cnd, &o));
  int64_t h = 0;
  TRACE(candle_high(cnd, &h));
  int64_t l = 0;
  TRACE(candle_low(cnd, &l));
  int64_t c = 0;
  TRACE(candle_close(cnd, &c));

  if (o == h && c == l && h != l) {
    *res = SINGLE_CANDLE_PATTERN_BLACK_MARUBOZU;
  }

  return RISKI_ERROR_CODE_NONE;
}
