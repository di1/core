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
double linear_equation_eval(struct linear_equation* eq, int64_t z) {
  return (eq->slope * z) + eq->intercept;
}

enum LINEAR_EQUATION_DIRECTION linear_equation_direction(
    struct linear_equation* eq, int64_t z) {
  (void)eq;
  (void)z;
  // TODO impliment this
  return IS_EQUAL;
}

// Frees the linear equation
void linear_equation_free(struct linear_equation** eq) {
  free(*eq);
  *eq = NULL;
}
