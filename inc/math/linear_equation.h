#ifndef LINEAR_EQUATION_
#define LINEAR_EQUATION_

#include <stdlib.h>

/*
 * Defines a linear equation.
 *
 * This linear equation has a domain in N but a range of R
 */
struct linear_equation;

enum LINEAR_EQUATION_DIRECTION { IS_BELOW = 0, IS_ABOVE = 1, IS_EQUAL = 2 };

/*
 * Creates a new linear equation from two points
 * @param {int64_t} x1 The x coordiant of point 1
 * @param {int64_t} y1 The y coordiant of point 1
 * @param {int64_t} x2 The x coordiant of point 2
 * @param {int64_t} y2 The y coordiant of point 2
 */
struct linear_equation* linear_equation_new(int64_t x1, int64_t y1, int64_t x2,
                                            int64_t y2);

/*
 * Evaluates a linear equation at a point
 * @param {struct linear_equation*} eq The equation
 * @param {int64_t} z The point to evaluate at
 * @return {double} The Y coordinate at z.
 */
int64_t linear_equation_eval(struct linear_equation* eq, int64_t z);

/*
 * Frees a linear equation allocated by linear_equation_new
 * @param {struct linear_equation**} eq The equation to free
 */
void linear_equation_free(struct linear_equation** eq);

#endif
