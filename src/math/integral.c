#include <math/integral.h>

enum RISKI_ERROR_CODE integral_chart(struct chart* cht, size_t a, size_t b,
                                     double* res) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  COMPARISON_CHECK(a, b, <, RISKI_ERROR_CODE_COMPARISON_FAIL, RISKI_ERROR_TEXT);

  double summation = 0;

  struct candle* c = NULL;
  int64_t close = 0;
  
  TRACE(chart_get_candle(cht, a, &c)); 
  TRACE(candle_close(c, &close));
  summation += close / (double)2.0;

  for (size_t i = a+1; i <= b-1; ++i) {
    TRACE(chart_get_candle(cht, i, &c));
    TRACE(candle_close(c, &close));
    summation += ((double)close/2.0);
  }

  TRACE(chart_get_candle(cht, b, &c)); 
  TRACE(candle_close(c, &close));
  summation += (double)(close / 2.0);

  // TODO: check for overflow maybe?
  
  double integral = (double)(summation) / (double)2.0;
  *res = integral;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE integral_line(struct linear_equation* eq, size_t a,
                                    size_t b, double* res) {
  PTR_CHECK(eq, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  COMPARISON_CHECK(a, b, <, RISKI_ERROR_CODE_COMPARISON_FAIL, RISKI_ERROR_TEXT);

  size_t width = b-a; 
  int64_t tb = linear_equation_eval(eq, b);
  int64_t ta = linear_equation_eval(eq, a);

  double integral = (double)width * ((double)tb + ((double)(ta-tb)/2.0));

  *res = integral;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE integral_const(size_t c, size_t a, size_t b, double* res) {
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  COMPARISON_CHECK(a, b, <, RISKI_ERROR_CODE_COMPARISON_FAIL, RISKI_ERROR_TEXT);
  

  *res = (double)c * (double)(b - a);

  return RISKI_ERROR_CODE_NONE;
}
