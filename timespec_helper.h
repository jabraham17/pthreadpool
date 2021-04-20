
#ifndef _TIMESPEC_HELPER_H_
#define _TIMESPEC_HELPER_H_

#include <time.h>

// given an amount in s and ns, place the values in the timespec
void th_calc_timespec(struct timespec* ts, unsigned long long s,
                      unsigned long long ns);
// convert timespec to ns
unsigned long long th_conv_timespec_ns(struct timespec* ts);
// convert timespec to us
float th_conv_timespec_us(struct timespec* ts);
// convert timespec to ms
float th_conv_timespec_ms(struct timespec* ts);
// convert timespec to s
float th_conv_timespec_s(struct timespec* ts);

// a - b
void th_sub_timespec(struct timespec* ts, struct timespec* a,
                     struct timespec* b);
void th_add_timespec(struct timespec* ts, struct timespec* a,
                     struct timespec* b);

unsigned long long th_gettime_ns();
float th_gettime_ms();
#endif
