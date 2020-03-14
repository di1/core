#ifndef HORIZONTAL_LINE_
#define HORIZONTAL_LINE_

#ifndef CANDLE_
#include <chart/candle.h>
#else
struct candle;
#endif

#ifndef CHART_
#include <chart/chart.h>
#else
struct chart;
#endif

#include <log/log.h>

void find_horizontal_line(struct chart* cht, size_t end_candle_index);

#endif
