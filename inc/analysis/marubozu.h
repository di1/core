#ifndef MARUBOZU_
#define MARUBOZU_

#include <chart/candle.h>
#include <analysis/enumations.h>

#include <stdbool.h>

enum SINGLE_CANDLE_PATTERNS is_black_marubozu(struct candle* c);
enum SINGLE_CANDLE_PATTERNS is_white_marubozu(struct candle* c);

#endif
