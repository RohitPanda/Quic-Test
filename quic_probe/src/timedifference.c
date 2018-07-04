//
// Created by sergei on 08.06.18.
//

#include <time.h>
#include "timedifference.h"

/*
 * or is timespec1 greater timespec2
 * 1 timespec1 greater timespec2
 * 0 timespec1 less timespec2
 * -1 even
 */
int compare_timespec(struct timespec* timespec1, struct timespec* timespec2)
{
    if (timespec1->tv_sec > timespec2->tv_sec)
        return 1;
    if (timespec1->tv_sec < timespec2->tv_sec)
        return 0;
    if (timespec1->tv_nsec > timespec2->tv_nsec)
        return 1;
    if (timespec1->tv_nsec < timespec2->tv_nsec)
        return 0;
    return -1;
}

void fill_time_difference(struct timespec* timespec1, struct timespec* timespec2, struct timespec* difference_out)
{
    struct timespec* greater_timer;
    struct timespec* less_time;
    if(compare_timespec(timespec1, timespec2))
    {
        greater_timer = timespec1;
        less_time = timespec2;
    } else
    {
        greater_timer = timespec2;
        less_time = timespec1;
    }

    difference_out->tv_sec = greater_timer->tv_sec - less_time->tv_sec;
    if (greater_timer->tv_nsec < less_time->tv_nsec)
    {
        difference_out->tv_sec--;
        difference_out->tv_nsec = 1000000000UL - less_time->tv_nsec + greater_timer->tv_nsec;
    } else
        difference_out->tv_nsec = greater_timer->tv_nsec - less_time->tv_nsec;
}

struct timespec get_time_difference(struct timespec* timespec1, struct timespec* timespec2)
{
    struct timespec difference;
    fill_time_difference(timespec1, timespec2, &difference);
    return difference;
}

void fill_time_difference_with_now(struct timespec* timespec, struct timespec* difference_out)
{
    struct timespec now_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now_time);
    return fill_time_difference(&now_time, timespec, difference_out);
}

struct timespec get_time_difference_with_now(struct timespec* timespec)
{
    struct timespec difference;
    fill_time_difference_with_now(timespec, &difference);
    return difference;
}
