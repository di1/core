ADD_SUBDIRECTORY(chart)
ADD_SUBDIRECTORY(book)
ADD_SUBDIRECTORY(iex)
ADD_SUBDIRECTORY(security)
ADD_SUBDIRECTORY(exchange)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(analysis)
ADD_SUBDIRECTORY(math)
ADD_SUBDIRECTORY(oanda)
ADD_SUBDIRECTORY(cjson)

ADD_LIBRARY(string_builder string_builder.c)
ADD_LIBRARY(error_codes error_codes.c)
ADD_LIBRARY(logger logger.c)

SET(CMAKE_ENABLE_EXPORTS TRUE)

ADD_EXECUTABLE(riski main.c)
TARGET_LINK_LIBRARIES(
    riski cjson string_builder oanda logger book iex chart security exchange
        server math analysis Threads::Threads OpenSSL::SSL OpenSSL::Crypto 
        ${CMAKE_DL_LIBS})
