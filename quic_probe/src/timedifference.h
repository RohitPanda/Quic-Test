//
// Created by sergei on 08.06.18.
//

#ifndef QUIC_PROBE_TIMEDIFFERENCE_H
#define QUIC_PROBE_TIMEDIFFERENCE_H

#include <sys/time.h>

void fill_time_difference(struct timespec* timespec1, struct timespec* timespec2,
                          struct timespec* difference_out);

struct timespec get_time_difference(struct timespec* timespec1, struct timespec* timespec2);

void fill_time_difference_with_now(struct timespec* timespec, struct timespec* difference_out);

struct timespec get_time_difference_with_now(struct timespec* timespec);

#endif //QUIC_PROBE_TIMEDIFFERENCE_H
