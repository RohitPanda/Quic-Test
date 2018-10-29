cmake_minimum_required(VERSION 3.10)
project(youtube_tcp_test)

set(CMAKE_C_STANDARD 11)

set(LIB_DIR  ${CMAKE_CURRENT_LIST_DIR}/../build)

include_directories(${LIB_DIR}/boringssl/include src)
include_directories(${LIB_DIR}/curl-7.58.0/build/include src)
include_directories(${LIB_DIR}/ffmpeg-4.0/build/include src)
include_directories(${LIB_DIR}/libtask src)
include_directories(/usr/local/include/nghttp2 src)

find_library(AVFORMAT libavformat.a HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(AVCODEC libavcodec.a HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
find_library(AVUTIL libavutil.a HINTS ${LIB_DIR}/ffmpeg-4.0/build/lib REQUIRED)
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
INSTALL(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/build/youtube_tcp_test
	   DESTINATION bin)
    
   SET(MAJOR_VERSION "1")
   SET(MINOR_VERSION "0")
   SET(PATCH_VERSION "0")
    
   IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
	   INCLUDE(InstallRequiredSystemLibraries)
	    
	   SET(CPACK_SET_DESTDIR "on")
	   SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
	   SET(CPACK_GENERATOR "DEB")
	    
	   SET(CPACK_PACKAGE_DESCRIPTION "Youtube test TCP")
	   SET(CPACK_PACKAGE_VENDOR "Rohit Panda")
	   SET(CPACK_PACKAGE_CONTACT "rohit.panda@tum.de")
	   SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
	   SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
	   SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
	   SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
	   SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
	    
	    
	   SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
	   SET(CPACK_DEBIAN_PACKAGE_SECTION "kde")
	   SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
	    
	   SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)
	   INCLUDE(CPack)
	    
   ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
   
target_link_libraries(youtube_tcp_test ${CURL} ${AVFORMAT} ${AVCODEC} ${AVUTIL} ${SSL} ${CRYPTO} ${NGHTTP2} z m pthread dl)
