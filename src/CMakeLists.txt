ADD_SUBDIRECTORY(chart)
ADD_SUBDIRECTORY(book)
ADD_SUBDIRECTORY(iex)
ADD_SUBDIRECTORY(log)
ADD_SUBDIRECTORY(security)
ADD_SUBDIRECTORY(exchange)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(analysis)
ADD_SUBDIRECTORY(fxpig)
ADD_SUBDIRECTORY(math)

ADD_LIBRARY(string_builder string_builder.c)

ADD_EXECUTABLE(riski main.c)
TARGET_LINK_LIBRARIES(riski
  string_builder fxpig log book iex chart security exchange server math analysis Threads::Threads)
