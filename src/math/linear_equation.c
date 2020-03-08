#include <math/linear_equation.h>

// Defines a linear equation
struct linear_equation {
  // m
  double slope;

  // b
  double intercept;
};

struct linear_equation* linear_equation_new(int64_t x1, int64_t y1, int64_t x2,
                                            int64_t y2) {
  double slope = (y2 - y1) / (x2 - x1);

  // y - (mx) = b
  double intercept = y1 - (slope * x1);

  struct linear_equation* eq =
      (struct linear_equation*)malloc(1 * sizeof(struct linear_equation));

  eq->slope = slope;
  eq->intercept = intercept;

  return eq;
}

// Evaluates mx+b and returns y
int64_t linear_equation_eval(struct linear_equation* eq, int64_t z) {
  return (int64_t) ((eq->slope * z) + eq->intercept);
}

enum LINEAR_EQUATION_DIRECTION linear_equation_direction(
    struct linear_equation* eq, int64_t z, int64_t y) {

  if (linear_equation_eval(eq, z) < y)
    return IS_BELOW;
  else if (linear_equation_eval(eq, z) > y)
    return IS_ABOVE;
  else
    return IS_EQUAL;
}

// Frees the linear equation
void linear_equation_free(struct linear_equation** eq) {
  free(*eq);
  *eq = NULL;
}
