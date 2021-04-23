
#ifndef _TIMESPEC_HELPER_H_
#define _TIMESPEC_HELPER_H_

#include <time.h>

// given an amount in s and ns, place the values in the timespec
static inline void th_calc_timespec(struct timespec* ts, unsigned long long s,
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
static inline unsigned long long th_conv_timespec_ns(struct timespec* ts) {
    return ts->tv_nsec +
           (unsigned long long)ts->tv_sec * 1000ull * 1000ull * 1000ull;
}
// convert timespec to us
static inline float th_conv_timespec_us(struct timespec* ts) {
    return (float)ts->tv_nsec / 1000.0f + (float)ts->tv_sec * 1000.0f * 1000.0f;
}
// convert timespec to ms
static inline float th_conv_timespec_ms(struct timespec* ts) {
    return (float)ts->tv_nsec / 1000.0f / 1000.0f + (float)ts->tv_sec * 1000.0f;
}
// convert timespec to s
static inline float th_conv_timespec_s(struct timespec* ts) {
    return (float)ts->tv_nsec / 1000.0f / 1000.0f / 1000.0f + (float)ts->tv_sec;
}

// a - b
static inline void th_sub_timespec(struct timespec* ts, struct timespec* a,
                     struct timespec* b) {
    unsigned long long diff = th_conv_timespec_ns(a) - th_conv_timespec_ns(b);
    th_calc_timespec(ts, 0, diff);
}
static inline void th_add_timespec(struct timespec* ts, struct timespec* a,
                     struct timespec* b) {
    unsigned long long sum = th_conv_timespec_ns(a) + th_conv_timespec_ns(b);
    th_calc_timespec(ts, 0, sum);
}

static inline unsigned long long th_gettime_ns() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return th_conv_timespec_ns(&t);
}
static inline float th_gettime_ms() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return th_conv_timespec_ms(&t);
}

#endif
