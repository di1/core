ADD_LIBRARY(analysis trend_line.c horizontal_line.c marubozu.c
  spinning_top.c doji.c analysis.c)

TARGET_LINK_LIBRARIES(analysis chart)
