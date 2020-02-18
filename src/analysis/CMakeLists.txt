ADD_LIBRARY(analysis marubozu.c
  spinning_top.c doji.c analysis.c)

TARGET_LINK_LIBRARIES(analysis chart)
