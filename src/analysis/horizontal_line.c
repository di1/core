#include <analysis/horizontal_line.h>

enum RISKI_ERROR_CODE horizontal_line_analysis(struct chart* cht,
                                               size_t num_candles,
                                               int64_t compare_price,
                                               enum DIRECTION direction) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  /*
   * Loop backwards from the end candles high/close, if three prices
   * (high/close) before this candle had the same price and all data
   * between the left-most and the end candle closed above/below the price
   * then it is a valid support line. Continue looking back until this
   * isn't valid. The left most candle that has this property will be the start
   * of the trend.
   */

  size_t num_confirmations = 0;
  size_t last_valid_confirmation = num_candles - 1;
  for (int i = (int)(num_candles)-2; i >= 0; --i) {
    struct candle* current_candle = NULL;
    TRACE(chart_get_candle(cht, i, &current_candle));

    // Check if the candle closes above the latest_high_price
    // break out because we have looked to far
    int64_t cur_candle_close = 0;
    TRACE(candle_close(current_candle, &cur_candle_close));

    if (direction == DIRECTION_RESISTANCE) {
      if (cur_candle_close > compare_price) break;
    } else if (direction == DIRECTION_SUPPORT) {
      if (cur_candle_close < compare_price) break;
    }

    // If the open/high/low/close == latest_high_price then
    // count this candle in the support/resistance line.
    //
    // This is valid because the close is lower or equal
    // to the support/resistance line we are trying to find

    uint64_t cur_candle_volume = 0;
    int64_t cur_candle_open = 0;
    int64_t cur_candle_high = 0;
    int64_t cur_candle_low = 0;

    TRACE(candle_volume(current_candle, &cur_candle_volume));
    TRACE(candle_high(current_candle, &cur_candle_high));
    TRACE(candle_low(current_candle, &cur_candle_low));
    TRACE(candle_open(current_candle, &cur_candle_open));

    if (cur_candle_volume > 0 &&
        (cur_candle_high == compare_price || cur_candle_open == compare_price ||
         cur_candle_close == compare_price ||
         cur_candle_low == compare_price)) {
      num_confirmations += 1;
      last_valid_confirmation = i;
    }
  }

  if (num_confirmations >= 3) {
    TRACE(chart_put_horizontal_line_pattern(cht, last_valid_confirmation,
                                            num_candles - 1, direction));
  }

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE find_horizontal_line(struct chart* cht,
                                           size_t num_candles) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // Analysis requires at least 3 candles
  if (num_candles < 3) return RISKI_ERROR_CODE_NONE;

  // Find horizontal prices from the candle high
  struct candle* cur_candle = NULL;
  TRACE(chart_get_candle(cht, num_candles - 1, &cur_candle));

  int64_t cur_candle_high = 0;
  TRACE(candle_high(cur_candle, &cur_candle_high));

  TRACE(horizontal_line_analysis(cht, num_candles, cur_candle_high,
                                 DIRECTION_RESISTANCE));

  int64_t cur_candle_low = 0;
  TRACE(candle_low(cur_candle, &cur_candle_low));
  // Find horizontal prices from the candle low
  TRACE(horizontal_line_analysis(cht, num_candles, cur_candle_low,
                                 DIRECTION_SUPPORT));

  return RISKI_ERROR_CODE_NONE;
}
