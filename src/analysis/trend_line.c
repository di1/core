#include <analysis/trend_line.h>

void find_trend_line(struct chart* cht, size_t num_candles) {
  if (num_candles < 3) return;

  size_t slope_first_point = num_candles - 1;

  struct candle* candle_first = chart_get_candle(cht, slope_first_point);

  for (int i = (int)(num_candles)-2; i >= 0; --i) {
    size_t slope_second_point = i;

    struct candle* candle_second = chart_get_candle(cht, slope_second_point);

    // We don't deal with horizontal lines thats horizontal_line.c job,
    // the trend lines we will find are sloped
    if (candle_high(candle_second) == candle_high(candle_first)) continue;

    // Build linear equation from the two points
    struct linear_equation* eq =
        linear_equation_new(slope_first_point, candle_high(candle_first),
                            slope_second_point, candle_high(candle_second));

    size_t number_of_confirmations = 0;
    size_t last_valid_confirmation = 0;

    // Verify that from the second point to the first point everything is below
    // (or equal) to the line
    for (int j = (int)slope_first_point; j >= 0; --j) {
      enum LINEAR_EQUATION_DIRECTION led = linear_equation_direction(
          eq, j, candle_close(chart_get_candle(cht, j)));
      switch (led) {
        case LINEAR_EQUATION_DIRECTION_ABOVE:
          goto continue_outer_loop;
        case LINEAR_EQUATION_DIRECTION_EQUAL:
          last_valid_confirmation = j;
          number_of_confirmations += 1;
          continue;
        case LINEAR_EQUATION_DIRECTION_BELOW:
          if (linear_equation_direction(
                  eq, j, candle_high(chart_get_candle(cht, j))) ==
              LINEAR_EQUATION_DIRECTION_EQUAL) {
            last_valid_confirmation = j;
            number_of_confirmations += 1;
          }
          continue;
      }
    }

    // Two confirmations at least from the two points that were used to
    // construct the line one more for it to be valid (thats what the great
    // economics said don't ask me why)
    if (number_of_confirmations >= 3) {
      log_debug("number of confirmations %lu, %lu %lu", number_of_confirmations,
                slope_second_point, last_valid_confirmation);
      chart_put_sloped_line_pattern(cht, last_valid_confirmation,
                                    slope_first_point, DIRECTION_RESISTANCE);
    }
  continue_outer_loop:
    linear_equation_free(&eq);
  }
}
