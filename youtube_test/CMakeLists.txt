cmake_minimum_required(VERSION 3.10)
project(youtube_test)

set(CMAKE_C_STANDARD 11)
set(LIB_DIR  ${CMAKE_CURRENT_LIST_DIR}/../build)

find_library(AVFORMAT avformat HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(AVCODEC avcodec HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(AVUTIL avutil HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(SSL ssl HINTS ${LIB_DIR}/boringssl/lib REQUIRED)
find_library(CRYPTO crypto HINTS ${LIB_DIR}/boringssl/lib REQUIRED)

find_library(LSQUIC lsquic HINTS ${LIB_DIR}/lsquic-client/src/liblsquic REQUIRED)
find_library(QUIC_CLIENT quic_client HINTS ../quic_probe REQUIRED)
find_library(EVENT libevent.a REQUIRED)

set(FFMPEG_LIBS ${AVFORMAT} ${AVCODEC} ${AVUTIL})
set(ENCRYPTION_LIB ${SSL} ${CRYPTO})
set(QUIC_LIB ${QUIC_CLIENT} ${LSQUIC})

include_directories(${LIB_DIR}/ffmpeg-4.0/build/include src)
include_directories(${LIB_DIR}/boringssl/include src)
include_directories(${LIB_DIR}/lsquic-client/include src)
include_directories(../quic_probe/include src)

add_executable(youtube_test
        src/adaptive.c
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

target_link_libraries(youtube_test ${AVFORMAT} ${AVCODEC} ${AVUTIL} ${QUIC_CLIENT} ${LSQUIC} ${EVENT} pthread ${SSL} ${CRYPTO} z m )
