#include <chart/chart.h>

struct chart {
  // the interval between the two candles in nanoseconds
  uint64_t interval; 

  // the number of candles _slots_ that are pre allocated
  size_t num_candles_allocated;

  // the current candle
  size_t cur_candle;

  // the list of candles 
  struct candle** candles;
};

struct chart* chart_new(uint64_t interval) {
  (void) interval;
  return NULL;
}
