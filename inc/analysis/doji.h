#ifndef DOJI_
#define DOJI_

#include <analysis/enumations.h>

// if guiard circular dependency
#ifndef CANDLE_
#include <chart/candle.h>
#else
struct candle;
#endif

enum SINGLE_CANDLE_PATTERNS is_doji_dragonfly(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_gravestone(struct candle* cnd);
enum SINGLE_CANDLE_PATTERNS is_doji_generic(struct candle* cnd);

#endif
