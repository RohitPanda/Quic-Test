cmake_minimum_required(VERSION 3.10)
project(youtube_tcp_test)

set(CMAKE_C_STANDARD 11)

set(LIB_DIR  ${CMAKE_CURRENT_LIST_DIR}/../build)

include_directories(${LIB_DIR}/boringssl/include src)
include_directories(${LIB_DIR}/curl-7.58.0/build/include src)
include_directories(${LIB_DIR}/ffmpeg-4.0/build/include src)
include_directories(${LIB_DIR}/libtask src)
include_directories(/usr/local/include/nghttp2 src)

find_library(AVFORMAT avformat HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(AVCODEC avcodec HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(AVUTIL avutil HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(SSL ssl HINTS ${LIB_DIR}/boringssl/lib REQUIRED)
find_library(CRYPTO crypto HINTS ${LIB_DIR}/boringssl/lib REQUIRED)
find_library(CURL curl HINTS ${LIB_DIR}/curl-7.58.0/build/lib REQUIRED)
find_library(NGHTTP2 nghttp2 HINTS /usr/local/lib REQUIRED)


add_executable(youtube_tcp_test
        src/adaptive.c
        src/curlops.c
        src/getinfo.c
        src/helper.c
        src/metrics.c
        src/mm_parser.c
        src/youtube-dl.c
        src/arguments_parser.c
        src/arguments_parser.h
        src/network_addresses.c
        src/network_addresses.h)

target_link_libraries(youtube_tcp_test ${CURL} ${AVFORMAT} ${AVCODEC} ${AVUTIL} ${SSL} ${CRYPTO} ${NGHTTP2} z m pthread dl)
