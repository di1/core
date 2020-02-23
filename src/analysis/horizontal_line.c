#include <analysis/horizontal_line.h>

#include "chart/chart.h"

void horizontal_line_analysis(struct chart* cht, size_t num_candles,
                              int64_t compare_price, enum DIRECTION direction) {
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
    struct candle* current_candle = chart_get_candle(cht, i);

    // Check if the candle closes above the latest_high_price
    // break out because we have looked to far
    if (direction == DIRECTION_RESISTANCE) {
      if (candle_close(current_candle) > compare_price) break;
    } else if (direction == DIRECTION_SUPPORT) {
      if (candle_close(current_candle) < compare_price) break;
    }

    // If the open/high/low/close == latest_high_price then
    // count this candle in the support/resistance line.
    //
    // This is valid because the close is lower or equal
    // to the support/resistance line we are trying to find
    if (candle_volume(current_candle) > 0 &&
        (candle_high(current_candle) == compare_price ||
         candle_open(current_candle) == compare_price ||
         candle_close(current_candle) == compare_price ||
         candle_low(current_candle) == compare_price)) {
      num_confirmations += 1;
      last_valid_confirmation = i;
    }
  }

  if (num_confirmations >= 3) {
    chart_put_trend_line_pattern(cht, last_valid_confirmation, num_candles - 1,
                                 direction);
  }
}

void find_horizontal_line(struct chart* cht, size_t num_candles) {
  // Analysis requires at least 3 candles
  if (num_candles < 3) return;

  // Find horizontal prices from the candle high
  horizontal_line_analysis(cht, num_candles,
                           candle_high(chart_get_candle(cht, num_candles - 1)),
                           DIRECTION_RESISTANCE);

  // Find horizontal prices from the candle low
  horizontal_line_analysis(cht, num_candles,
                           candle_low(chart_get_candle(cht, num_candles - 1)),
                           DIRECTION_SUPPORT);
}
