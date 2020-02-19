#ifndef DOJI_
#define DOJI_

#include <chart/candle.h>
#include <chart/chart.h>
#include <stdlib.h>

enum SINGLE_CANDLE_PATTERNS is_doji_dragonfly(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_gravestone(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_generic(struct candle* cnd);

#endif
