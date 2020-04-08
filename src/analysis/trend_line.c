#include <analysis/trend_line.h>

enum RISKI_ERROR_CODE find_trend_line(struct chart* cht, size_t num_candles,
                                      enum DIRECTION type) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if (num_candles < 3) return RISKI_ERROR_CODE_NONE;

  enum RISKI_ERROR_CODE (*candle_get_init)(struct candle * cnd, int64_t * res);

  switch (type) {
    case DIRECTION_RESISTANCE:
      candle_get_init = &candle_high;
      break;
    case DIRECTION_SUPPORT:
      candle_get_init = &candle_low;
      break;
    default:
      return RISKI_ERROR_CODE_UNKNOWN;
  }

  size_t slope_first_point = num_candles - 1;

  struct candle* candle_first = NULL;
  TRACE(chart_get_candle(cht, slope_first_point, &candle_first));

  size_t best_last_valid_confirmation = 0;
  size_t best_last_valid_confirmation_coverage = 0;
  size_t best_number_of_confirmations = 0;

  struct linear_equation* best_eq = NULL;
  struct linear_equation* eq = NULL;

  for (int i = (int)(num_candles)-2; i >= 0; --i) {
    size_t slope_second_point = i;

    struct candle* candle_second = NULL;
    TRACE(chart_get_candle(cht, slope_second_point, &candle_second));

    // We don't deal with horizontal lines thats horizontal_line.c job,
    // the trend lines we will find are sloped
    int64_t high_low_second = 0;
    int64_t high_low_first = 0;
    TRACE(candle_get_init(candle_second, &high_low_second));
    TRACE(candle_get_init(candle_first, &high_low_first));
    if (high_low_second == high_low_first) continue;

    // Build linear equation from the two points
    eq = linear_equation_new(slope_first_point, high_low_first,
                             slope_second_point, high_low_second);

    size_t number_of_confirmations = 0;
    size_t last_valid_confirmation = 0;

    // Verify that from the second point to the first point everything is below
    // (or equal) to the line
    for (int j = (int)slope_first_point; j >= 0; --j) {
      struct candle* cur_candle = NULL;
      TRACE(chart_get_candle(cht, j, &cur_candle));
      int64_t cur_candle_close = 0;
      int64_t cur_candle_high_low = 0;
      TRACE(candle_close(cur_candle, &cur_candle_close));
      TRACE(candle_get_init(cur_candle, &cur_candle_high_low));

      enum LINEAR_EQUATION_DIRECTION led =
          linear_equation_direction(eq, j, cur_candle_close);

      // make sure this candle doesn't break the support/resistance line
      switch (type) {
        case DIRECTION_RESISTANCE:
          switch (led) {
            case LINEAR_EQUATION_DIRECTION_ABOVE:
              goto continue_outer_loop;
              break;
            default:
              break;
          }
          break;
        case DIRECTION_SUPPORT:
          switch (led) {
            case LINEAR_EQUATION_DIRECTION_BELOW:
              goto continue_outer_loop;
              break;
            default:
              break;
          }
          break;
        default:
          return RISKI_ERROR_CODE_UNKNOWN;
      }

      led = linear_equation_direction(eq, j, cur_candle_high_low);
      if (led == LINEAR_EQUATION_DIRECTION_EQUAL) {
        last_valid_confirmation = j;
        number_of_confirmations += 1;
      }

      /*
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
      */
    }
  continue_outer_loop:
    // Two confirmations at least from the two points that were used to
    // construct the line one or two more for it to be valid
    // (thats what the great economics said don't ask me why)
    if (number_of_confirmations >= 4) {
      // check the length
      size_t segment_coverage = slope_first_point - last_valid_confirmation;
      if (segment_coverage > best_last_valid_confirmation) {
        best_last_valid_confirmation_coverage = segment_coverage;
        best_last_valid_confirmation = last_valid_confirmation;
        best_number_of_confirmations = number_of_confirmations;
        // linear_equation_free(&best_eq);
        if (best_eq) linear_equation_free(&best_eq);
        best_eq = eq;
      } else {
        linear_equation_free(&eq);
      }
    } else {
      linear_equation_free(&eq);
    }
  }

  if (best_number_of_confirmations >= 4 &&
      best_last_valid_confirmation_coverage > 3) {
    char* n = NULL;
    TRACE(chart_get_name(cht, &n));

    TRACE(logger_analysis(
        n, "SLOPED_TREND", __func__, __FILENAME__, __LINE__,
        "confirmations=%lu first_point=%lu last_point=%lu "
        "coverage=%lu type=%s",
        best_number_of_confirmations, slope_first_point,
        best_last_valid_confirmation, best_last_valid_confirmation_coverage,
        (type == DIRECTION_SUPPORT) ? "SUPPORT" : "RESISTANCE"));
    TRACE(chart_put_sloped_line_pattern(cht, best_last_valid_confirmation,
                                        slope_first_point, type, 0));
    linear_equation_free(&best_eq);
  }

  return RISKI_ERROR_CODE_NONE;
}
