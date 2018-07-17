cmake_minimum_required(VERSION 3.9)
project(quic_probe)
set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")

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
set(GENERAL_LIBS lsquic event pthread ssl crypto z m)

include_directories(src)
include_directories(src/test)
include_directories(include)
include_directories(../build/lsquic-client/include)
include_directories(../build/boringssl/include)
include_directories(../build/libevent-2.1.8-stable/build/include)

link_directories("../build/lsquic-client/src/liblsquic")
link_directories("../build/boringssl/lib")
link_directories("../build/libevent-2.1.8-stable/build/lib")

##create a library

add_library(quic_client STATIC
    ${GENERAL_SOURCES}
)

##create an executable
#
add_executable(quic_probe src/main.c)
target_link_libraries(quic_probe quic_client ${GENERAL_LIBS})

