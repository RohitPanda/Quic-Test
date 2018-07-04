//
// Created by sergei on 29.06.18.
//

#ifndef YOUTUBE_TEST_TIMESPEC_CONVERTOR_H_
#define YOUTUBE_TEST_TIMESPEC_CONVERTOR_H_

#include <sys/types.h>

double timespec_to_seconds(struct timespec* time);
void timespec_diff(struct timespec *greater_time, struct timespec *less_time, struct timespec *difference_out);
double get_time_spec_diff(struct timespec* timespec1, struct timespec* timespec2);
long long timespec_to_usec(struct timespec* time);

#endif //YOUTUBE_TEST_TIMESPEC_CONVERTOR_H_