#ifndef CHART_
#define CHART_

// public enumations for analysis results
#include <analysis/enumations.h>

// if guard circular dependency
#ifndef ANALYSIS_
#include <analysis/analysis.h>
#endif

#include <chart/candle.h>
#include <log/log.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <tracer.h>

/*
 * Private chart struct
 */
struct chart;

/*
 * Creates a new chart given the interval of data
 * @param {uint64_t} interval The length of each candle, this time unit can be
 * anything as long as the ts used in the chart_update function is in the same
 * units as the chart_new interval.
 * @param {char*} name A name for the chart to be identifyed as.
 * @param {struct chart**} cht A pointer to the resulting struct pointer
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_new(uint64_t interval, char* name,
                                struct chart** cht);

/*
 * Frees a given chart. And sets *cht to NULL on success
 * @param {struct chart**} cht A pointer to an allocated chart.
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_free(struct chart** cht);

/*
 * Updates the chart given the chart, price, and timestamp
 * of the price data.
 * @param {struct chart*} cht The chart to update
 * @param {int64_t} price The new price
 * @param {uint64_t} ts The time this price happened, must be in the same
 * time format used in chart_new interval variable.
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_update(struct chart* cht, int64_t price,
                                   uint64_t ts);

/*
 * Sets *name to the name of the chart
 * @param {struct chart*} cht A chart
 * @param {char**} A pointer to where to store the resulting name
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_get_name(struct chart* cht, char** name);

/*
 * Converts the chart to a json object and sets *json to the json string
 * @param {struct chart*} cht A chart
 * @param {char**} json A place to set the json string ptr
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_json(struct chart* cht, char** json);

/*
 * Gets the latest candle update as a json
 * @param {struct chart* cht} A chart
 * @param {char**} json A place to set the json string ptr
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_latest_candle(struct chart* cht, char** json);

/*
 * Aquires the analysis lock mutex, (blocking)
 * @param {struct chart*} A chart
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_analysis_lock(struct chart* cht);

/*
 * Releases the analysis lock, (blocking)
 * @param {struct chart*} cht A chart
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_analysis_unlock(struct chart* cht);

/*
 * Sets a candle tag with a single candle pattern
 * @param {struct chart*} cht A chart
 * @param {size_t} index The index of the candle to flag
 * @param {enum SINGLE_CANDLE_PATTERNS} identifier The flag to assign
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_put_single_candle_pattern(
    struct chart* cht, size_t index, enum SINGLE_CANDLE_PATTERNS identifier);

/*
 * Marks a line on a chart given two candles, and a boolean "direction"
 * which indicates weather this line should be draw from the lows or the highs.
 * @param {struct chart*} cht A chart
 * @param {size_t} start The start index
 * @param {size_t} end The end index
 * @param {enum DIRECTION} The direction of the line
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_put_horizontal_line_pattern(
    struct chart* cht, size_t start, size_t end, enum DIRECTION direction);

/*
 * Invalidates trends that are currently broken. Will loop through the trend
 * lines found and find the trend lines that have been broken. These trend
 * lines are marked as invalid and will not be considered when joining trend
 * lines together.
 * @param {struct chart*} cht A chart
 * @param {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_invalidate_trends(struct chart* cht);

/*
 * Sets *json to a json representing the analysis of the chart.
 * @param {struct chart*} cht A chart
 * @param {char**} json A place to store the json result pointer
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_analysis_json(struct chart* cht, char** json);

/*
 * Returns a candle, this will only return finalized candles. And will cause
 * stack exception if a caller attempts to get an unfinalized candle.
 * @param {struct chart*} cht A chart
 * @param {size_t} index The candle index to obtain
 * @param {struct candle**} cnd A place to store the candle pointer
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_get_candle(struct chart* cht, size_t index,
                                       struct candle** cnd);

/*
 * Adds a sloped line pattern to the chart representation.
 * @param {struct chart*} cht The chart
 * @param {size_t} start The start of the trend line (should be less than end)
 * @param {size_t} end The end of the trend line (should be greater than start)
 * @param {enum DIRECTION} direction Weather or not this is a suport or
 * resistance line
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE chart_put_sloped_line_pattern(struct chart* cht,
                                                    size_t start, size_t end,
                                                    enum DIRECTION direction);

#endif
