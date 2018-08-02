cmake_minimum_required(VERSION 3.9)
project(quic_probe)
set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
set(LIB_DIR  ${CMAKE_CURRENT_LIST_DIR}/../build)

set(GENERAL_SOURCES
        src/address_finder.c
        src/address_finder.h
        src/connection_establisher.c
        src/connection_establisher.h
        src/connection_init_handler.c
        src/connection_init_handler.h
        src/connection_read_handler.c
        src/connection_read_handler.h
        src/connection_write_handler.c
        src/connection_write_handler.h
        src/engine_structs.c
        src/engine_structs.h
        src/quic_engine_holder.c
        src/quic_engine_holder.h
        src/socket_handler.c
        src/socket_handler.h
        src/connection_params.c
        src/connection_params.h
        src/socket_buffer.c
        src/socket_buffer.h
        src/quic_downloader.c
        src/stack.c
        src/stack.h
        src/cycle_buffer.c
        src/cycle_buffer.h
        src/simple_list.c
        src/simple_list.h
        src/event_handlers.c
        src/event_handlers.h
        src/error_report.c
        src/error_report.h
        src/text.c
        src/text.h
        src/args_data.c
        include/args_data.h
        include/quic_downloader.h
        src/html_header_parser.c
        src/html_header_parser.h
        src/mono_parser.c
        src/mono_parser.h
        src/timedifference.c
        src/timedifference.h src/queue.c src/queue.h)

include_directories(src)
include_directories(src/test)
include_directories(include)
include_directories(../build/lsquic-client/include)
include_directories(../build/boringssl/include)

find_library(SSL ssl HINTS ${LIB_DIR}/boringssl/lib REQUIRED)
find_library(CRYPTO crypto HINTS ${LIB_DIR}/boringssl/lib REQUIRED)
find_library(LSQUIC lsquic HINTS ${LIB_DIR}/lsquic-client/src/liblsquic REQUIRED)
find_library(EVENT libevent.a REQUIRED)

message(${EVENT})

set(GENERAL_LIBS ${LSQUIC} ${EVENT} pthread ${SSL} ${CRYPTO} z m)

##create a library

add_library(quic_client STATIC
    ${GENERAL_SOURCES}
)

##create an executable
#
add_executable(quic_probe src/main.c)
target_link_libraries(quic_probe quic_client ${GENERAL_LIBS})

