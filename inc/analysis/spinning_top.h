#ifndef SPINNING_TOP_
#define SPINNING_TOP_

#include <analysis/enumations.h>
#include <chart/candle.h>

/*
 * Checks if the given candle is a black marubozu
 * @param {struct candle*} c The candle to check
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE is_white_spinning_top(struct candle *cnd,
                                            enum SINGLE_CANDLE_PATTERNS *res);

/*
 * Checks if the given candle is a black marubozu
 * @param {struct candle*} c The candle to check
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE is_black_spinning_top(struct candle *cnd,
                                            enum SINGLE_CANDLE_PATTERNS *res);

#endif
