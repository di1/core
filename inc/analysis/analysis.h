#ifndef ANALYSIS_
#define ANALYSIS_

// riski error codes to enable stack tracing
#include <error_codes.h>
#include <tracer.h>

// used for candle interface
#include <chart/candle.h>

// if guard circular dependency
#ifndef CHART_
#include <chart/chart.h>
#else
struct chart;
#endif

// include the different analysis

// used for function call tracing
#include <tracer.h>

// standard inputs
#include <logger.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

// dlopen and dynamic stuff to load up libraries
#include <dlfcn.h>

// used to find all the files in a folder
#include <dirent.h>

#include <api.h>
#include <math/linear_equation.h>
#include <stdatomic.h>
#include <time.h> // for clock_t

/*
 * Private struct holding the needed information to perform an analysis
 */
struct analysis_info;

/*
 * A cdll used for pushing and popping analysis to the threads
 */
struct analysis_list;

/*
 * Initalizes threads to perform analysis on.
 * @return The status
 */
enum RISKI_ERROR_CODE analysis_init(void);

/*
 * Pushes information to a thread to analyize it
 * @param sec The chart to perform analysis on
 * @param start The minimum candle to look at
 * @param end The maximum candle to look at
 * @return The status
 */
enum RISKI_ERROR_CODE analysis_push(struct chart *sec, size_t start,
                                    size_t end);

/*
 * Pops the very first element to the analysis and puts it inside (*inf).
 * @param bin The thread bin to pop from
 * @param inf A pointer to populate the pop result.
 * @return The status
 */
enum RISKI_ERROR_CODE analysis_pop(struct analysis_list *bin,
                                   struct analysis_info **inf);

/*
 * Joins the analysis threads together and cleans up loose memory
 * @return The status
 */
enum RISKI_ERROR_CODE analysis_cleanup(void);

/*
 * Creates a new analysis info that can be processed by a worker thread.
 * @param cht The chart to analyize
 * @param start The start of the analysis
 * @param end The end of the analysis
 * @param inf Pointer to the resulting analysis info
 */
enum RISKI_ERROR_CODE analysis_create_info(struct chart *cht, size_t start,
                                           size_t end,
                                           struct analysis_info **inf);

extern int ANALYSIS_INTERRUPED;

#endif
