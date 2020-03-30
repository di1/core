#ifndef INTEGRAL_
#define INTEGRAL_

#include <chart/candle.h>
#include <chart/chart.h>
#include <error_codes.h>
#include <math/linear_equation.h>
#include <tracer.h>

/*
 * Computes the integral of a chart between a, b
 * @param {struct chart*} cht The chart to integrate
 * @param {size_t} a The lower bound
 * @param {size_t} b The upper bound
 * @param {double*} res Will set *res to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE integral_chart(struct chart* cht, size_t a, size_t b,
                                     double* res);

/*
 * Compute the integral of a line between a, b
 * @param {struct linear_equation*} eq The line to integrate
 * @param {size_t} a The lower bound
 * @param {size_t} b The upper bound
 * @param {double*} res Will set *res to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE integral_line(struct linear_equation* eq, size_t a,
                                    size_t b, double* res);

/*
 * Compute the integral of a horizontal line
 * @param {size_t} c The c in y=c (must be positive)
 * @param {size_t} a The lower bound
 * @param {size_t} b The upper bound
 * @param {double*} res Will set *res to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE integral_const(size_t c, size_t a, size_t b, double* res);

#endif
