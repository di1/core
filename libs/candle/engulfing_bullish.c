#include <api.h>

const char* name = "Bullish Engulfing";
const char* author = "Vittorio Papandrea";

char*
get_name()
{
  return (char*) name;
}

char*
get_author()
{
  return (char*) author;
}

enum RISKI_ERROR_CODE
run (struct chart *cht, size_t idx)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if (idx < 2)
    {
      return RISKI_ERROR_CODE_NONE;
    }
  struct candle *cnd1 = NULL;
  TRACE (chart_get_candle (cht, idx - 1, &cnd1));

  struct candle *cnd2 = NULL;
  TRACE (chart_get_candle (cht, idx - 2, &cnd2));

  int64_t cnd1_o, cnd1_h, cnd1_l, cnd1_c = 0;
  TRACE (candle_open (cnd1, &cnd1_o));
  TRACE (candle_high (cnd1, &cnd1_h));
  TRACE (candle_low (cnd1, &cnd1_l));
  TRACE (candle_close (cnd1, &cnd1_c));

  // make sure this candle is going up
  if (cnd1_c <= cnd1_o)
    {
      return RISKI_ERROR_CODE_NONE;
    }

  int64_t cnd2_o, cnd2_h, cnd2_l, cnd2_c = 0;
  TRACE (candle_open (cnd2, &cnd2_o));
  TRACE (candle_high (cnd2, &cnd2_h));
  TRACE (candle_low (cnd2, &cnd2_l));
  TRACE (candle_close (cnd2, &cnd2_c));

  // make sure this candle is going down
  if (cnd2_c >= cnd2_o)
    {
      return RISKI_ERROR_CODE_NONE;
    }

  // compute body size
  size_t cnd1_body_size = cnd1_c - cnd1_o;
  size_t cnd2_body_size = cnd2_o - cnd2_c;

  // make sure the first body is longer than the second candle
  if (cnd1_body_size <= cnd2_body_size)
    {
      return RISKI_ERROR_CODE_NONE;
    }

  // make sure the first candle "engulfes" the second candle
  if (cnd1_o < cnd2_c && cnd1_c > cnd1_o && cnd1_l < cnd2_l && cnd1_h > cnd2_h)
    {
      char* sec_name = NULL;
      TRACE(chart_get_name(cht, &sec_name));

      logger_analysis(sec_name, name, __func__,
          __FILENAME__, __LINE__, "%s" , " ");

      struct analysis_result *res =
        (struct analysis_result*) malloc(sizeof(struct analysis_result) * 1);
      PTR_CHECK(res, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

      res->type = CANDLE_PATTERN;
      
      struct candle_pattern *data = malloc(sizeof(struct candle_pattern) * 1);
      data->candles_spanning = 2;
      data->short_code = strdup("BE");

      res->draw_data = (void*) data;
      TRACE(chart_put_analysis(cht, idx-1,res));
    }

  return RISKI_ERROR_CODE_NONE;
}

struct vtable exports = {
  get_name,
  get_author,
  run
};
