
// benchmark using this to optimize a loop

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "pool.h"
#include "timespec_helper.h"
#include "timing.h"

// baseline kernel
__attribute__((always_inline)) float dot(float* x, float* y, size_t n) {
    float sum = 0;
    for(size_t i = 0; i < n; i++) {
        sum += x[i] * y[i];
    }
    return sum;
}

#define NUM_THREADS 4

// openmp implementation
__attribute__((always_inline)) float dot_openmp(float* x, float* y, size_t n) {
    float sum = 0;
#pragma omp parallel for reduction(+ : sum) num_threads(NUM_THREADS)
    for(size_t i = 0; i < n; i++) {
        sum += x[i] * y[i];
    }
    return sum;
}

typedef struct {
    float* x;
    float* y;
    size_t low;
    size_t high;
    float* sum;
} dot_pthreadpool_arg_t;

__attribute__((always_inline)) void* dot_pthreadpool_worker(void* arg) {
    dot_pthreadpool_arg_t* parg = (dot_pthreadpool_arg_t*)arg;
    *(parg->sum) = 0;
    for(size_t i = parg->low; i < parg->high; i++) {
        *(parg->sum) += parg->x[i] * parg->y[i];
    }
    return NULL;
}

// my pthreadpool implementation
__attribute__((always_inline)) float dot_pthreadpool(pool_t* pool, float* x, float* y, size_t n) {
    float sums[NUM_THREADS];
    dot_pthreadpool_arg_t args[NUM_THREADS];
    task_t* threads[NUM_THREADS];
    size_t step = n / NUM_THREADS;
    for(int i = 0; i < NUM_THREADS; i++) {
        args[i].x = x;
        args[i].y = y;
        args[i].sum = sums + i;
        args[i].low = step * i;
        args[i].high = step * i + step;
    }
    args[NUM_THREADS - 1].high = n; // make the last one take any remainders

    for(int i = 0; i < NUM_THREADS; i++) {
        threads[i] = pool_submit(pool, dot_pthreadpool_worker, (void*)(args+i), NULL);
    }

    float sum = 0;
    for(int i = 0; i < NUM_THREADS; i++) {
        pool_wait(&threads[i]);
        sum += sums[i];
    }
    return sum;
}




typedef struct {
    float *x;
    float *y;
    float *result;
    size_t n;
    pool_t* pool;
} test_args_t;

void* bench_dot(void* arg) {
    test_args_t* targ = (test_args_t*)arg;
    *(targ->result) = dot(targ->x, targ->y, targ->n);
    return NULL;
}
void* bench_openmp(void* arg) {
    test_args_t* targ = (test_args_t*)arg;
    *(targ->result) = dot_openmp(targ->x, targ->y, targ->n);
    return NULL;
}
void* bench_pthreadpool(void* arg) {
    test_args_t* targ = (test_args_t*)arg;
    *(targ->result) = dot_pthreadpool(targ->pool, targ->x, targ->y, targ->n);
    return NULL;
}


#define rand_range_int(low, high) low + rand() % (high + 1 - low)
#define rand_range_float(low, high) (float)(rand_range_int(low, high))

int main(__attribute((unused)) int argc, __attribute((unused)) char **argv) {

    srand(time(NULL));

    size_t n = 1024 * 4; // each array is a 1/4 page

    float result;
    float *x = (float *)aligned_alloc(64, n * sizeof(float));
    float *y = (float *)aligned_alloc(64, n * sizeof(float));

    pool_t* pool = pool_init(NUM_THREADS);

    test_args_t arg = {.x = x, .y = y, .result = &result, .n = n, .pool = pool};

    for(size_t i = 0; i < n; i++) {
        x[i] = rand_range_float(1, 200);
        y[i] = rand_range_float(1, 200);
    }

    printf("Baseline   : ");
    benchmark(bench_dot, (void *)(&arg), 50, 5000, 0b01, NULL);
    printf("OPENMP     : ");
    benchmark(bench_openmp, (void *)(&arg), 50, 5000, 0b01, NULL);
    printf("PThreadPool: ");
    benchmark(bench_pthreadpool, (void *)(&arg), 50, 5000, 0b01, NULL);

    // accuracy check
    printf("\nAccuracy\n");
    bench_dot((void *)(&arg));
    printf("B   : %16.6f\n", *(arg.result));
    bench_openmp((void *)(&arg));
    printf("OMP : %16.6f\n", *(arg.result));
    bench_pthreadpool((void *)(&arg));
    printf("PTP: %16.6f\n", *(arg.result));
}
