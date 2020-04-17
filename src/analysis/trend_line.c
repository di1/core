#include <analysis/trend_line.h>

#include "chart/chart.h"
#include "math/linear_equation.h"

enum RISKI_ERROR_CODE get_working_value(struct candle* cnd, int64_t* res,
                                        enum DIRECTION type) {
  switch (type) {
    case DIRECTION_SUPPORT:
      TRACE(candle_low(cnd, res));
      break;
    case DIRECTION_RESISTANCE:
      TRACE(candle_high(cnd, res));
      break;
    default:
      return RISKI_ERROR_CODE_UNKNOWN;
  }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE find_trend_line(struct chart* cht, size_t num_candles,
                                      enum DIRECTION type) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if (num_candles < 3) return RISKI_ERROR_CODE_NONE;

  /*
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
  */

  size_t slope_first_point = num_candles - 1;

  struct candle* candle_first = NULL;
  TRACE(chart_get_candle(cht, slope_first_point, &candle_first));
  int64_t candle_first_working_value = 0;
  TRACE(get_working_value(candle_first, &candle_first_working_value, type));

  int64_t max_confirmation_width = num_candles / 3;
  for (int64_t w = max_confirmation_width; w >= 1; --w) {
    int64_t confirmation_working_value = 0;
    struct candle* confirmation_candle = NULL;
    TRACE(chart_get_candle(cht, num_candles - w - 1, &confirmation_candle));
    TRACE(get_working_value(confirmation_candle, &confirmation_working_value,
                            type));

    // not my job don't care
    if (confirmation_working_value == candle_first_working_value) {
      continue;
    }

    // Create a line between the last candle and last candle - width
    struct linear_equation* eq = linear_equation_new(
        slope_first_point, candle_first_working_value,
        slope_first_point - w - 1, confirmation_working_value);

    bool trend_broken = false;

    size_t num_indirect_confirmations = 0;
    size_t num_confirmations = 0;
    size_t last_valid_confirmation = 0;

    for (int64_t c = slope_first_point - w - 1; c >= 0 && !trend_broken;
         c -= w) {
      // Check if the expected confirmation point is true
      // Check a radius for a confirmation
      // The radius should be a radius of w/n, as n <= w where when n = w,
      // only the perfectly distributed trends will be shown.
      // Making the radius relient on w allows for larger perfect
      // trend lines to have a greater margin of error
      bool at_least_one_confirmation = false;
      int64_t confirmation_point = 0;
      for (int64_t r = c - w / 2; r <= c + w / 2 && (size_t)r < num_candles;
           ++r) {
        struct candle* possible_confirmation = NULL;
        TRACE(chart_get_candle(cht, r, &possible_confirmation));
        int64_t possible_confirmation_val = 0;
        TRACE(get_working_value(possible_confirmation,
                                &possible_confirmation_val, type));

        if (linear_equation_direction(eq, r, possible_confirmation_val) ==
            LINEAR_EQUATION_DIRECTION_EQUAL) {
          at_least_one_confirmation = true;
          confirmation_point = r;
          break;
        }
      }

      if (!at_least_one_confirmation) {
        trend_broken = true;
        break;
      }

      // Check the data inbetween the last two confirmation points
      for (int64_t i = confirmation_point;
           i < (int64_t)num_candles && i < (int64_t) num_candles && !trend_broken;
           ++i) {
        struct candle* cnd = NULL;
        TRACE(chart_get_candle(cht, i, &cnd));
        int64_t working_value = 0;
        TRACE(candle_close(cnd, &working_value));
        enum LINEAR_EQUATION_DIRECTION dir =
            linear_equation_direction(eq, i, working_value);

        switch (dir) {
          case LINEAR_EQUATION_DIRECTION_ABOVE:
            switch (type) {
              case DIRECTION_SUPPORT:
                continue;
                break;
              case DIRECTION_RESISTANCE:
                trend_broken = true;
                goto dont_confirm;
                continue;
                break;
              default:
                return RISKI_ERROR_CODE_NONE;
            }
            break;
          case LINEAR_EQUATION_DIRECTION_BELOW:
            switch (type) {
              case DIRECTION_SUPPORT:
                trend_broken = true;
                goto dont_confirm;
                continue;
                break;
              case DIRECTION_RESISTANCE:
                continue;
                break;
              default:
                return RISKI_ERROR_CODE_NONE;
            }
            break;
          case LINEAR_EQUATION_DIRECTION_EQUAL:
            num_indirect_confirmations += 1;
            break;
        }
      }

      num_confirmations += 1;
      last_valid_confirmation = confirmation_point;
    }
dont_confirm:
    // print the number of confirmations along with the width
    if (num_confirmations >= 3) {
      TRACE(chart_put_sloped_line_pattern(cht, last_valid_confirmation,
                                          slope_first_point, type, 0));

      char* n = NULL;
      TRACE(chart_get_name(cht, &n));
      printf("[%s] width=%lu num_confirmation=%lu indirect=%lu\n", n, w,
             num_confirmations, num_indirect_confirmations);

      free(eq);
      break;
    }

    free(eq);
  }

  return RISKI_ERROR_CODE_NONE;
}
