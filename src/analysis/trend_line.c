#include <analysis/trend_line.h>

void find_trend_line(struct chart* cht, size_t num_candles) {
  log_debug("ummmmmmm");
  if (num_candles < 3) return;

  size_t slope_first_point = num_candles - 1;

  struct candle* candle_first = chart_get_candle(cht, slope_first_point);

  for (int i = (int)(num_candles)-2; i >= 0; --i) {
    size_t slope_second_point = i;

    struct candle* candle_second = chart_get_candle(cht, slope_second_point);


    // We don't deal with horizontal lines thats horizontal_line.c job,
    // the trend lines we will find are sloped
    if (candle_high(candle_second) == candle_high(candle_first))
      continue;

    // Build linear equation from the two points
    struct linear_equation* eq = linear_equation_new(slope_first_point,
        candle_high(candle_first), slope_second_point, candle_high(candle_second));


    size_t number_of_confirmations = 0;

    // Verify that from the second point to the first point everything is below (or equal) to the line
    for (size_t j = slope_second_point; j <= slope_first_point; ++j) {
      enum LINEAR_EQUATION_DIRECTION led = linear_equation_direction(eq, j, candle_close(chart_get_candle(cht, j)));
      switch (led) {
        case LINEAR_EQUATION_DIRECTION_ABOVE:
          goto continue_outer_loop;
          break;
        case LINEAR_EQUATION_DIRECTION_EQUAL:
          number_of_confirmations += 1;
          break;
        case LINEAR_EQUATION_DIRECTION_BELOW:
          break;
      }
    }
continue_outer_loop:
    log_debug("number of confirmations %lu", number_of_confirmations);
    linear_equation_free(&eq);
  }
}
