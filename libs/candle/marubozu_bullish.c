#include "chart/chart.h"
#include "logger.h"
#include <api.h>

const char* name = "Bullish Marubozu";
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
run(struct chart* cht, size_t idx)
{
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR,
      RISKI_ERROR_TEXT);

  struct candle *cnd = NULL;
  TRACE(chart_get_candle(cht, idx-1, &cnd));

  int64_t o = 0;
  TRACE (candle_open (cnd, &o));
  int64_t h = 0;
  TRACE (candle_high (cnd, &h));
  int64_t l = 0;
  TRACE (candle_low (cnd, &l));
  int64_t c = 0;
  TRACE (candle_close (cnd, &c));

  // we check l != h to make sure we don't have a doji
  if (o == l && c == h && l != h)
    {
      // we have a marubozu here
      struct analysis_result *res =
        (struct analysis_result*) malloc(sizeof(struct analysis_result) * 1);
      PTR_CHECK(res, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

      res->type = CANDLE_PATTERN;
      
      struct candle_pattern *data = malloc(sizeof(struct candle_pattern) * 1);
      data->candles_spanning = 1;
      data->short_code = strdup("M");

      res->draw_data = (void*) data;

      char* sec_name = NULL;
      TRACE(chart_get_name(cht, &sec_name));

      logger_analysis(sec_name, name, __func__,
          __FILENAME__, __LINE__, "%s" , " ");
      TRACE(chart_put_analysis(cht, idx-1,res));
    }

  return RISKI_ERROR_CODE_NONE;
}

struct vtable exports = {
  get_name,
  get_author,
  run
};
