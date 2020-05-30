#ifndef LOGGER_
#define LOGGER_

#include <error_codes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifndef TRACER_
#include <tracer.h>
#endif

/*
 * Logs items in the category of "info".
 * @param {char*} func The originating function
 * @param {char*} filename The filename provided by __FILENAME__ (in tracer.h)
 * @param {int} line The line number
 * @param {char*} fmt The format like printf
 * @param {va_list} ... Arguments to give to printf
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE logger_info (const char *func, const char *filename,
                                   int line, const char *fmt, ...);

/*
 * Logs items in the category of "info".
 * @param {char*} func The originating function
 * @param {char*} filename The filename provided by __FILENAME__ (in tracer.h)
 * @param {int} line The line number
 * @param {char*} fmt The format like printf
 * @param {va_list} ... Arguments to give to printf
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE logger_warning (const char *func, const char *filename,
                                      int line, const char *fmt, ...);

/*
 * Logs items in the category of "error".
 * @param {enum RISKI_ERROR_CODE} err The error code to return
 * @param {char*} func The originating function
 * @param {char*} filename The filename provided by __FILENAME__ (in tracer.h)
 * @param {int} line The line number
 * @param {char*} fmt The format like printf
 * @param {va_list} ... Arguments to give to printf
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE logger_error (enum RISKI_ERROR_CODE err,
                                    const char *func, const char *filename,
                                    int line, const char *fmt, ...);

/*
 * Logs items in the category of "analysis".
 * @param {const char*} security The security this analysis refers to.
 * @param {const char*} analysis_name A name for the analysis
 * @param {char*} func The originating function
 * @param {char*} filename The filename provided by __FILENAME__ (in tracer.h)
 * @param {int} line The line number
 * @param {char*} fmt The format like printf
 * @param {va_list} ... Arguments to give to printf
 * @return {enum RISKI_ERROR_CODE} The status
 */

enum RISKI_ERROR_CODE logger_analysis (const char *security,
                                       const char *analysis_name,
                                       const char *func, const char *filename,
                                       int line, const char *fmt, ...);

#endif
