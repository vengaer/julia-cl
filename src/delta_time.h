#ifndef DELTA_TIME_H
#define DELTA_TIME_H

#include <stddef.h>

#include <sys/time.h>

static inline void delta_tick(struct timeval *start, struct timeval *stop, float *out_delta) {
    gettimeofday(stop, NULL);
    *out_delta = ((stop->tv_sec - start->tv_sec) * 1000000.f + stop->tv_usec - start->tv_usec) / 1000.f;
    *start = *stop;
}

#endif
