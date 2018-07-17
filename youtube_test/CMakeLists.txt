cmake_minimum_required(VERSION 3.10)
project(youtube_test)

set(CMAKE_C_STANDARD 11)
set(LSQUIC_CLIENT_LIBS quic_client lsquic event pthread ssl crypto z m)
set(GENERAL_LIBS ${LSQUIC_CLIENT_LIBS})
set(LOCAL_LIBS avformat avcodec avutil)

include_directories(../build/ffmpeg-4.0/build/include src)
include_directories(../build/boringssl/include src)
include_directories(../build/lsquic-client/include src)
include_directories(../quic_probe/include src)

link_directories(../build/boringssl/lib)
link_directories(../build/lsquic-client/src/liblsquic)
link_directories(../build/ffmpeg-4.0/build/lib)
link_directories(../quic_probe)
link_directories(../build/libevent-2.1.8-stable/build/lib)

add_executable(youtube_test
        src/adaptive.c
        src/coro.c
        src/download_ops.c
        src/getinfo.c
        src/helper.c
        src/metrics.c
        src/mm_parser.c
        src/youtube-dl.c
        src/arguments_parser.c
        src/arguments_parser.h
        src/network_addresses.c
        src/network_addresses.h
        src/timespec_convertor.c
        src/timespec_convertor.h)
target_link_libraries(youtube_test ${LOCAL_LIBS} ${GENERAL_LIBS} )
