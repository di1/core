#ifndef SPINNING_TOP_
#define SPINNING_TOP_

#include <chart/candle.h>

#include <analysis/enumations.h>

enum SINGLE_CANDLE_PATTERNS is_white_spinning_top(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_black_spinning_top(struct candle* cnd);

#endif
