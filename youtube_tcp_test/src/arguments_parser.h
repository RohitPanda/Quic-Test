//
// Created by sergei on 15.06.18.
//

#include <stdbool.h>
#include <sys/types.h>
#include <limits.h>

#ifndef YOUTUBE_TEST_ARGUMENTS_PARSER_H
#define YOUTUBE_TEST_ARGUMENTS_PARSER_H



#define MAXURLLENGTH 512

#define LEN_CHUNK_MINIMUM 5 /*Shortest length of the buffer*/


enum IPv {IPvSYSTEM, IPv4, IPv6};

struct program_arguments {
    /*print instantaneous output through progress function if true*/
    bool instantaneous_output;
    enum IPv ip_version;
    int max_bitrate;
    int min_test_time;
    int max_test_time;
    /*
     * If false, terminate current transfer if a stall occurs and try downloading a lower bit rate
     * If true, Continue downloading the same bit rate video even when stall occurs.
     */
    bool one_bitrate;
    int buffer_len;
};

int set_arguments(int argc, char **argv, char *youtubelink, struct program_arguments* arguments_out);

#endif //YOUTUBE_TEST_ARGUMENTS_PARSER_H