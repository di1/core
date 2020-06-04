ADD_LIBRARY(analysis analysis.c)

TARGET_LINK_LIBRARIES(analysis chart math Threads::Threads)
