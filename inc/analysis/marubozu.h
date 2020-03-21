#ifndef MARUBOZU_
#define MARUBOZU_

#include <analysis/enumations.h>
#include <chart/candle.h>
#include <error_codes.h>
#include <stdbool.h>
#include <tracer.h>

/*
 * Checks if the given candle is a black marubozu
 * @param {struct candle*} c The candle to check
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE is_black_marubozu(struct candle* c,
                                        enum SINGLE_CANDLE_PATTERNS* res);

/*
 * Checks if the given candle is a white marubozu
 * @param {struct candle*} c The candle to check
 * @param {enum SINGLE_CANDLE_PATTERN*} res Sets it to the result
 * @return {enum RISKI_ERROR_CODE} The status
 */

enum RISKI_ERROR_CODE is_white_marubozu(struct candle* c,
                                        enum SINGLE_CANDLE_PATTERNS* res);

#endif
