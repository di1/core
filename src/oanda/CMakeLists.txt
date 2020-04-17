ADD_LIBRARY(oanda oanda.c request_builder.c)
TARGET_LINK_LIBRARIES(oanda cjson)
