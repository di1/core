#include <api.h>

static const char* name = "Bearish Marubozu";
static const char* author = "Vittorio Papandrea";

const char* get_name() {
  return name;
}

const char* get_author() {
  return author;
}

enum RISKI_ERROR_CODE
run (struct chart *cht, size_t idx)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

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

  if (o == h && c == l && h != l)
    {
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
          FILENAME_SHORT, __LINE__, "%s" , " ");
      TRACE(chart_put_analysis(cht, idx-1,res));
    }

  return RISKI_ERROR_CODE_NONE;
}

struct vtable exports = {
  get_name,
  get_author,
  run
};
