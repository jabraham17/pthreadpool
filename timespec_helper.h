
#ifndef _TIMESPEC_HELPER_H_
#define _TIMESPEC_HELPER_H_

#include <time.h>

// given an amount in s and ns, place the values in the timespec
void th_calc_timespec(struct timespec *ts, unsigned long long s,
                   unsigned long long ns) {
    // we have to make sure we dont execde integer limits
    // or that we dont excede GNUC specs
    while(ns >= 1000ull * 1000ull * 1000ull) {
        s += 1;
        ns -= 1000ull * 1000ull * 1000ull;
    }
    ts->tv_sec = s;
    ts->tv_nsec = ns;
}
// convert timespec to ns
unsigned long long th_conv_timespec(struct timespec *ts) {
    return ts->tv_nsec + (unsigned long long)ts->tv_sec * 1000ull * 1000ull * 1000ull;
}
// subtract 2 timespecs
// a - b
void th_sub_timespec(struct timespec *ts, struct timespec *a, struct timespec *b) {
    unsigned long long diff = th_conv_timespec(a) - th_conv_timespec(b);
    th_calc_timespec(ts, 0, diff);
}
void th_add_timespec(struct timespec *ts, struct timespec *a, struct timespec *b) {
    unsigned long long diff = th_conv_timespec(a) + th_conv_timespec(b);
    th_calc_timespec(ts, 0, diff);
}

unsigned long long th_gettime() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return th_conv_timespec(&t);
}

#endif
