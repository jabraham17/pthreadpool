
#ifndef _TIMING_H_
#define _TIMING_H_
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define __LOOP(__N, __CODE)                                                    \
    for(size_t __i = 0; __i < __N; __i++) {                                    \
        __CODE                                                                 \
    }

#define __TIMESPEC_TO_MS(__TIMESPEC)                                           \
    (((float)__TIMESPEC.tv_sec * 1000.0) +                                     \
     ((float)__TIMESPEC.tv_nsec / 1000.0 / 1000.0))

// fix me to do convert to ms before difference
#define __TIME_IT(__FUNC, __LOOP_COUNT, __TIME)                                \
    {                                                                          \
        struct timespec __start, __end;                                        \
        clock_gettime(CLOCK_MONOTONIC, &__start);                              \
        __LOOP(__LOOP_COUNT, __FUNC;);                                         \
        clock_gettime(CLOCK_MONOTONIC, &__end);                                \
        __TIME = __TIMESPEC_TO_MS(__end) - __TIMESPEC_TO_MS(__start);          \
    }

#define __TIME_IT_PRINT(__FUNC, __LOOP_COUNT)                                  \
    {                                                                          \
        float time_spent;                                                      \
        __TIME_IT(__LOOP_COUNT, __FUNC, time_spent);                           \
        printf("Function: " #__FUNC ", Time: %4.5fms\n", time_spent);          \
    }

#define __TIME_IT_N(__FUNC, __LOOP_COUNT, __N, __TIMES)                        \
    for(size_t __i = 0; __i < __N; __i++) {                                    \
        __TIME_IT(__FUNC, __LOOP_COUNT, __TIMES[__i]);                         \
    }

#define __MEAN(__N, __SAMPLES, __M)                                            \
    {                                                                          \
        __M = 0;                                                               \
        for(size_t __i = 0; __i < __N; __i++) {                                \
            __M += __SAMPLES[__i];                                             \
        }                                                                      \
        __M = __M / __N;                                                       \
    }

#define __VARIANCE(__N, __SAMPLES, __V)                                        \
    {                                                                          \
        float __mean;                                                          \
        float __temp;                                                          \
        __MEAN(__N, __SAMPLES, __mean);                                        \
        __V = 0;                                                               \
        for(size_t __i = 0; __i < __N; __i++) {                                \
            __temp = (__SAMPLES[__i] - __mean);                                \
            __V += __temp * __temp;                                            \
        }                                                                      \
        __V = __V / (__N - 1);                                                 \
    }

// print options: 00 no print, 01 print mean/variance, 10 print all times, 11
// print both for 'return_times', only return if pointer is non null
__attribute((__noinline__)) static void benchmark(void* (*func)(void*), void* args,
                                           size_t sample_size,
                                           size_t loop_count, int print_flags,
                                           float** return_times) {

    float* times = (float*)malloc(sample_size * sizeof(float));
    float mean;
    float var;

    __TIME_IT_N(func(args), loop_count, sample_size, times);
    __MEAN(sample_size, times, mean);
    __VARIANCE(sample_size, times, var);

    if(print_flags & 1)
        printf("Mean: %8.4fms Variance: %8.4fms\n", mean, var);
    if(print_flags & 2) {
        size_t row_length = 5;
        for(size_t i = 0; i < sample_size;) {
            for(size_t j = 0; j < row_length && (j + i) < sample_size; j++) {
                printf("%8.4fms\t", times[j + i]);
            }
            i += row_length;
            printf("\n");
        }
    }

    // if we dont want the times, free it
    if(return_times != NULL) {
        *return_times = times;
    } else {
        free(times);
    }
}

#endif
