#ifndef CHART_
#define CHART_

// public enumations for analysis results
#include <analysis/enumations.h>

// if guard circular dependency
#ifndef ANALYSIS_
#include <analysis/analysis.h>
#endif

#include <chart/candle.h>
#include <logger.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string_builder.h>
#include <tracer.h>

// for CHAR_BIT
#include <limits.h>

/*
 * The struct to represent a trend line
 * @param {size_t} start_index The starting candle
 * @param {size_t} end_index The end index candle
 * @param {enum DIRECTION} direction The direction
 */
struct trend_line {
  size_t end_index;
  size_t start_index;
  enum DIRECTION direction;

  // There is an extra 4 bytes of usable memory in this struct
  char _p1[4];
};

/*
 * This represents a generic candle pattern
 * @param {size_t} candles_spanning The number of candles this
 * candle pattern spans. So for example a candle pattern that requires
 * three consecutive candles has a candles span of three.
 * @param {char*} short_code The short code is a string of size
 * candles_spanning that will be displayed along with the shaded region.
 */
struct candle_pattern {
  size_t candles_spanning;
  char *short_code;
};

/*
 * A NULL terminated linked list describing analysis that were
 * found.
 * @param {enum ANALYSIS_TYPE} The analysis type
 * @param {void*} draw_data This variable type is dependent on
 * the value of type. Below is a list of types and there assumed
 * draw_data data
 * @param {struct analysis_result*} next The next element in the list.
 * This is filled in by chart_put_analysis. Any data that was in there
 * will be replaced.
 *
 *
 * ANALYSIS_RESULT.CANDLE_PATTERN -> struct candle_pattern
 * ANALYSIS_RESULT.TREND_LINE -> struct trend_line
 */
struct analysis_result {
  void *draw_data;
  struct analysis_result *next;
  enum ANALYSIS_TYPE type;

  // There is an extra 4 bytes of usable memory in this struct
  char _p1[4];
};

/*
 * Private chart struct
 */
struct chart;

/*
 * Used to put an analysis result into the chart
 */
enum RISKI_ERROR_CODE chart_put_analysis(struct chart *cht, size_t idx,
                                         struct analysis_result *res);

/*
 * Creates a new chart given the interval of data
 * @param {uint64_t} interval The length of each candle, this time unit can be
 * anything as long as the ts used in the chart_update function is in the same
 * units as the chart_new interval.
 * @param {char*} name A name for the chart to be identifyed as.
 * @param {struct chart**} cht A pointer to the resulting struct pointer
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_new(uint64_t interval, char *name,
                                struct chart **cht);

/*
 * Frees a given chart. And sets *cht to NULL on success
 * @param {struct chart**} cht A pointer to an allocated chart.
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_free(struct chart **cht);

/*
 * Updates the chart given the chart, price, and timestamp
 * of the price data.
 * @param {struct chart*} cht The chart to update
 * @param {int64_t} price The new price
 * @param {uint64_t} ts The time this price happened, must be in the same
 * time format used in chart_new interval variable.
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_update(struct chart *cht, int64_t price,
                                   uint64_t ts);

/*
 * Sets *name to the name of the chart
 * @param {struct chart*} cht A chart
 * @param {char**} A pointer to where to store the resulting name
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_get_name(struct chart *cht, char **name);

/*
 * Converts the chart to a json object and sets *json to the json string
 * @param {struct chart*} cht A chart
 * @param {char**} json A place to set the json string ptr
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_json(struct chart *cht, char **json);

/*
 * Gets the latest candle update as a json
 * @param {struct chart* cht} A chart
 * @param {char**} json A place to set the json string ptr
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_latest_candle(struct chart *cht, char **json);

/*
 * Aquires the analysis lock mutex, (blocking)
 * @param {struct chart*} A chart
 * @return {enum RISKI_ERROR_CODE} The status
 */
// enum RISKI_ERROR_CODE chart_analysis_lock(struct chart *cht);

/*
 * Releases the analysis lock, (blocking)
 * @param {struct chart*} cht A chart
 * @return {enum RISKI_ERROR_CODE} The status
 */
// enum RISKI_ERROR_CODE chart_analysis_unlock(struct chart *cht);

/*
 * Sets a candle tag with a single candle pattern
 * @param {struct chart*} cht A chart
 * @param {size_t} index The index of the candle to flag
 * @param {enum SINGLE_CANDLE_PATTERNS} identifier The flag to assign
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE
chart_put_single_candle_pattern(struct chart *cht, size_t index,
                                enum SINGLE_CANDLE_PATTERNS identifier);

/*
 * Sets a candle tag with a single candle pattern
 * @param {struct chart*} cht A chart
 * @param {size_t} index The index of the candle to flag
 * @param {enum DOUBLE_CANDLE_PATTERNS} identifier The flag to assign
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE
chart_put_double_candle_pattern(struct chart *cht, size_t index,
                                enum DOUBLE_CANDLE_PATTERNS identifier);

/*
 * Marks a line on a chart given two candles, and a boolean "direction"
 * which indicates weather this line should be draw from the lows or the highs.
 * @param {struct chart*} cht A chart
 * @param {size_t} start The start index
 * @param {size_t} end The end index
 * @param {enum DIRECTION} The direction of the line
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE
chart_put_horizontal_line_pattern(struct chart *cht, size_t start, size_t end,
                                  enum DIRECTION direction);

/*
 * Invalidates trends that are currently broken. Will loop through the trend
 * lines found and find the trend lines that have been broken. These trend
 * lines are marked as invalid and will not be considered when joining trend
 * lines together.
 * @param {struct chart*} cht A chart
 * @param {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_invalidate_trends(struct chart *cht);

/*
 * Sets *json to a json representing the analysis of the chart.
 * @param {struct chart*} cht A chart
 * @param {char**} json A place to store the json result pointer
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_analysis_json(struct chart *cht, char **json);

/*
 * Returns a candle, this will only return finalized candles. And will cause
 * stack exception if a caller attempts to get an unfinalized candle.
 * @param {struct chart*} cht A chart
 * @param {size_t} index The candle index to obtain
 * @param {struct candle**} cnd A place to store the candle pointer
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_get_candle(struct chart *cht, size_t index,
                                       struct candle **cnd);

/*
 * Adds a sloped line pattern to the chart representation.
 * @param {struct chart*} cht The chart
 * @param {size_t} start The start of the trend line (should be less than end)
 * @param {size_t} end The end of the trend line (should be greater than start)
 * @param {enum DIRECTION} direction Weather or not this is a suport or
 * resistance line
 * @param {size_t} score The score of this trend line, [0,100]
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_put_sloped_line_pattern(struct chart *cht,
                                                    size_t start, size_t end,
                                                    enum DIRECTION direction,
                                                    size_t score);

#endif
