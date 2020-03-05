PROJECT(riski)

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message(FATAL_ERROR
      "In-source builds not allowed."
      "Please make a new directory (called a build directory) "
      "and run CMake from there. You may need to remove CMakeCache.txt.")
ENDIF()

IF(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

FIND_PACKAGE(PCAP REQUIRED)
FIND_PACKAGE(LibWebSockets REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ADD_COMPILE_OPTIONS("-g" "-O0" "-Wall" "-Wextra" "-Werror" "-Wpedantic")
ADD_COMPILE_DEFINITIONS(RUN_TESTS=$ENV{RUN_TESTS})

INCLUDE_DIRECTORIES(
  inc/
  /usr/include/
  /usr/local/include/
  ${PCAP_INCLUDE_DIRS}
  ${LIBWEBSOCKETS_INCLUDE_DIR}
)

LINK_DIRECTORIES(
  ${LIBWEBSOCKETS_LIBRARY_DIR}
  ${PCAP_LIBRARY_DIR}
)

ADD_SUBDIRECTORY(src/)

ADD_EXECUTABLE(riski src/main.c)
TARGET_LINK_LIBRARIES(riski ${PCAP_LIBRARY} ${LIBWEBSOCKETS_LIBRARIES}
  fxpig log book iex chart security exchange server math analysis
  Threads::Threads)
