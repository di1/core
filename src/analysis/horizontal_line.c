#include "chart/chart.h"
#include <analysis/horizontal_line.h>

void find_horizontal_line(struct chart* cht, size_t num_candles) {

  // Analysis requires at least 3 candles
  if (num_candles < 3)
    return;

  /*
   * Loop backwards from the end candles high/close, if three prices
   * (high/close) before this candle had the same price and all data
   * between the left-most and the end candle closed above/below the price
   * then it is a valid support line. Continue looking back until this
   * isn't valid. The left most candle that has this property will be the start
   * of the trend.
   */

  // Looking for resistance lines
  int64_t latest_high_price =
    candle_high(chart_get_candle(cht, num_candles-1));

  size_t num_confirmations = 0;
  size_t last_valid_confirmation = num_candles-1;
  for (int i = (int)(num_candles)-2; i >= 0; --i) {
    struct candle* current_candle = chart_get_candle(cht, i);

    // Check if the candle closes above the latest_high_price
    // break out because we have looked to far
    if (candle_close(current_candle) > latest_high_price)
      break;

    // If the open/high/low/close == latest_high_price then
    // count this candle in the support/resistance line.
    //
    // This is valid because the close is lower or equal
    // to the support/resistance line we are trying to find
    if (candle_volume(current_candle) > 0 &&
        (candle_high(current_candle) == latest_high_price ||
        candle_open(current_candle) == latest_high_price ||
        candle_close(current_candle) == latest_high_price ||
        candle_low(current_candle) == latest_high_price)) {
      num_confirmations += 1;
      last_valid_confirmation = i;
    }
  }


  if (num_confirmations >= 3) {
    log_debug("%s: found trend line spanning %lu candles with %lu confirmations",
        chart_get_name(cht),
        num_candles-last_valid_confirmation-1,num_confirmations);
    chart_put_trend_line_pattern(cht, last_valid_confirmation, num_candles-1, true);
  }

}
