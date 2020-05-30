#include <analysis/doji.h>

enum RISKI_ERROR_CODE
perform_doji_dragonfly (struct candle *cnd, enum SINGLE_CANDLE_PATTERNS *res)
{
  int64_t o = 0;
  TRACE (candle_open (cnd, &o));
  int64_t h = 0;
  TRACE (candle_high (cnd, &h));
  int64_t l = 0;
  TRACE (candle_low (cnd, &l));
  int64_t c = 0;
  TRACE (candle_close (cnd, &c));

  *res = SINGLE_CANDLE_PATTERN_NONE;
  if (o == c && o == h && l < o)
    {
      *res = SINGLE_CANDLE_PATTERN_DOJI_DRAGONFLY;
    }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
perform_doji_gravestone (struct candle *cnd, enum SINGLE_CANDLE_PATTERNS *res)
{
  int64_t o = 0;
  TRACE (candle_open (cnd, &o));
  int64_t h = 0;
  TRACE (candle_high (cnd, &h));
  int64_t l = 0;
  TRACE (candle_low (cnd, &l));
  int64_t c = 0;
  TRACE (candle_close (cnd, &c));

  *res = SINGLE_CANDLE_PATTERN_NONE;

  if (o == c && o == l && h > o)
    {
      *res = SINGLE_CANDLE_PATTERN_DOJI_GRAVESTONE;
    }

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
perform_doji_generic (struct candle *cnd, enum SINGLE_CANDLE_PATTERNS *res)
{
  int64_t o = 0;
  TRACE (candle_open (cnd, &o));
  int64_t h = 0;
  TRACE (candle_high (cnd, &h));
  int64_t l = 0;
  TRACE (candle_low (cnd, &l));
  int64_t c = 0;
  TRACE (candle_close (cnd, &c));

  *res = SINGLE_CANDLE_PATTERN_NONE;

  if (o == c && h != l)
    {
      *res = SINGLE_CANDLE_PATTERN_DOJI_GENERIC;
    }

  return RISKI_ERROR_CODE_NONE;
}
