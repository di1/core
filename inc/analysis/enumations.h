#ifndef ANALYSIS_ENUMERATIONS_
#define ANALYSIS_ENUMERATIONS_

#include <stdlib.h>

/**
 * enumation for candle patterns
 */
enum SINGLE_CANDLE_PATTERNS {
  SINGLE_CANDLE_PATTERN_NONE,
  SINGLE_CANDLE_PATTERN_WHITE_MARUBOZU,
  SINGLE_CANDLE_PATTERN_BLACK_MARUBOZU,
  SINGLE_CANDLE_PATTERN_WHITE_SPINNING_TOP,
  SINGLE_CANDLE_PATTERN_BLACK_SPINNING_TOP,
  SINGLE_CANDLE_PATTERN_DOJI_DRAGONFLY,
  SINGLE_CANDLE_PATTERN_DOJI_GRAVESTONE,
  SINGLE_CANDLE_PATTERN_DOJI_GENERIC
};

/**
 * enumeration of candle parts
 */
enum DIRECTION {
  DIRECTION_SUPPORT = 0,
  DIRECTION_RESISTANCE = 1,
  DIRECTION_INVALIDATED_SUPPORT = 2,
  DIRECTION_INVALIDATED_RESISTANCE = 3
};


#endif
