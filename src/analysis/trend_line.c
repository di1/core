#include <analysis/trend_line.h>

#include "chart/chart.h"

enum RISKI_ERROR_CODE find_trend_line(struct chart* cht, size_t num_candles) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if (num_candles < 3) return RISKI_ERROR_CODE_NONE;

  size_t slope_first_point = num_candles - 1;

  struct candle* candle_first = NULL;
  TRACE(chart_get_candle(cht, slope_first_point, &candle_first));

  size_t best_last_valid_confirmation = 0;
  size_t best_last_valid_confirmation_coverage = 0;
  size_t best_number_of_confirmations = 0;

  for (int i = (int)(num_candles)-2; i >= 0; --i) {
    size_t slope_second_point = i;

    struct candle* candle_second = NULL;
    TRACE(chart_get_candle(cht, slope_second_point, &candle_second));

    // We don't deal with horizontal lines thats horizontal_line.c job,
    // the trend lines we will find are sloped
    int64_t high_second = 0;
    int64_t high_first = 0;
    TRACE(candle_high(candle_second, &high_second));
    TRACE(candle_high(candle_first, &high_first));
    if (high_second == high_first) continue;

    // Build linear equation from the two points
    struct linear_equation* eq = linear_equation_new(
        slope_first_point, high_first, slope_second_point, high_second);

    size_t number_of_confirmations = 0;
    size_t last_valid_confirmation = 0;

    // Verify that from the second point to the first point everything is below
    // (or equal) to the line
    for (int j = (int)slope_first_point; j >= 0; --j) {
      struct candle* cur_candle = NULL;
      TRACE(chart_get_candle(cht, j, &cur_candle));
      int64_t cur_candle_close = 0;
      TRACE(candle_close(cur_candle, &cur_candle_close));
      enum LINEAR_EQUATION_DIRECTION led =
          linear_equation_direction(eq, j, cur_candle_close);
      switch (led) {
        case LINEAR_EQUATION_DIRECTION_ABOVE:
          goto continue_outer_loop;
        case LINEAR_EQUATION_DIRECTION_EQUAL:
          last_valid_confirmation = j;
          number_of_confirmations += 1;
          continue;
        case LINEAR_EQUATION_DIRECTION_BELOW: {
          int64_t cur_candle_high;
          TRACE(candle_high(cur_candle, &cur_candle_high));
          if (linear_equation_direction(eq, j, cur_candle_high) ==
              LINEAR_EQUATION_DIRECTION_EQUAL) {
            last_valid_confirmation = j;
            number_of_confirmations += 1;
          }
          continue;
        }
      }
    }

    // Two confirmations at least from the two points that were used to
    // construct the line one more for it to be valid (thats what the great
    // economics said don't ask me why)
    if (number_of_confirmations >= 4) {
      // check the length
      size_t segment_coverage = slope_first_point - last_valid_confirmation;
      if (segment_coverage > best_last_valid_confirmation) {
        best_last_valid_confirmation_coverage = segment_coverage;
        best_last_valid_confirmation = last_valid_confirmation;
        best_number_of_confirmations = number_of_confirmations;
      }

      // log_debug("number of confirmations %lu, %lu %lu",
      // number_of_confirmations,
      //          slope_second_point, last_valid_confirmation);
      // char* n = NULL;
      // TRACE(chart_get_name(cht, &n));

      // TRACE(logger_analysis(n, "SLOPED_TREND", __func__, __FILENAME__,
      // __LINE__,
      //                      "confirmations=%lu first_point=%lu
      //                      last_point=%lu", number_of_confirmations,
      //                      slope_first_point, last_valid_confirmation));
      // TRACE(chart_put_sloped_line_pattern(cht, last_valid_confirmation,
      //                                   slope_first_point,
      //                                   DIRECTION_RESISTANCE));
    }
  continue_outer_loop:
    linear_equation_free(&eq);
  }

  if (best_number_of_confirmations >= 4 &&
      best_last_valid_confirmation_coverage > 3) {
    char* n = NULL;
    TRACE(chart_get_name(cht, &n));

    TRACE(logger_analysis(
        n, "SLOPED_TREND", __func__, __FILENAME__, __LINE__,
        "confirmations=%lu first_point=%lu last_point=%lu coverage=%lu",
        best_number_of_confirmations, slope_first_point,
        best_last_valid_confirmation, best_last_valid_confirmation_coverage));
    TRACE(chart_put_sloped_line_pattern(cht, best_last_valid_confirmation,
                                        slope_first_point,
                                        DIRECTION_RESISTANCE));
  }

  return RISKI_ERROR_CODE_NONE;
}
