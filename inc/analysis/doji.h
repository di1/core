#ifndef DOJI_
#define DOJI_

#include <chart/chart.h>
#include <chart/candle.h>
#include <stdlib.h>

enum SINGLE_CANDLE_PATTERNS is_doji_top(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_middle(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_bot(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_generic(struct candle* cnd);

#endif
