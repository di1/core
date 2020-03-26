ADD_LIBRARY(candle candle.c)
ADD_LIBRARY(chart candle chart.c)

TARGET_LINK_LIBRARIES(chart analysis)
