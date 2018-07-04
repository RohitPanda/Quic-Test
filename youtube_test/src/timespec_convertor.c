//
// Created by sergei on 29.06.18.
//

#include "timespec_convertor.h"

#define NANOSECONDS_IN_SECOND_LONG 1000000000UL
#define NANOSECONDS_IN_SECOND_DOUBLE 1000000000.0


double timespec_to_seconds(struct timespec* time)
{
    return time->tv_sec + time->tv_nsec / NANOSECONDS_IN_SECOND_DOUBLE;
}

void timespec_diff(struct timespec *greater_time, struct timespec *less_time, struct timespec *difference_out)
{
    difference_out->tv_sec = greater_time->tv_sec - less_time->tv_sec;
    if (greater_time->tv_nsec < less_time->tv_nsec)
    {
        difference_out->tv_sec--;
        difference_out->tv_nsec = NANOSECONDS_IN_SECOND_LONG - less_time->tv_nsec + greater_time->tv_nsec;
    } else
        difference_out->tv_nsec = greater_time->tv_nsec - less_time->tv_nsec;
}

double get_time_spec_diff(struct timespec* timespec1, struct timespec* timespec2)
{
    struct timespec diff;
    timespec_diff(timespec1, timespec2, &diff);
    return timespec_to_seconds(&diff);
}

long long timespec_to_usec(struct timespec* time)
{
    return time->tv_sec * 1000000L + time->tv_nsec / 1000L;
}