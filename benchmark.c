
// benchmark using this to optimize a loop

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "pool.h"
#include "timespec_helper.h"
#include "timing.h"

// baseline kernel
double dot(double* x, double* y, size_t n) {
    double sum = 0;
    for(size_t i = 0; i < n; i++) {
        sum += x[i] * y[i];
    }
    return sum;
}

#ifndef NUM_THREADS
#define NUM_THREADS 6
#endif

// openmp implementation
double dot_openmp(double* x, double* y, size_t n) {
    double sum = 0;
    size_t i = 0;
    #pragma omp parallel private(i) num_threads(NUM_THREADS) 
    {
        #pragma omp for reduction(+ : sum)
        for(i = 0; i < n; i++) {
            sum += x[i] * y[i];
        }
    }
    return sum;
}

typedef struct {
    double* x;
    double* y;
    size_t low;
    size_t high;
    double* sum;
} dot_pthreadpool_arg_t;

void* dot_pthreadpool_worker(void* arg) {
    dot_pthreadpool_arg_t* parg = (dot_pthreadpool_arg_t*)arg;
    double sum = 0;
    for(size_t i = parg->low; i < parg->high; i++) {
        sum += parg->x[i] * parg->y[i];
    }
    *(parg->sum) = sum;
    return NULL;
}

// my pthreadpool implementation
double dot_pthreadpool(pool_t* pool, double* x, double* y, size_t n) {
    double sums[NUM_THREADS];
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

    double sum = 0;
    for(int i = 0; i < NUM_THREADS; i++) {
        DPRINTF("waiting on %d\n", i);
        pool_wait(&threads[i]);
        sum += sums[i];
    }
    return sum;
}




typedef struct {
    double *x;
    double *y;
    double *result;
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
#define rand_range_double(low, high) (double)(rand_range_int(low, high))

int main(__attribute((unused)) int argc, __attribute((unused)) char **argv) {

    srand(time(NULL));

    size_t n = 1024*1024;

    double result;
    double *x = (double *)aligned_alloc(64, n * sizeof(double));
    double *y = (double *)aligned_alloc(64, n * sizeof(double));

    pool_t* pool = pool_init(NUM_THREADS);

    test_args_t arg = {.x = x, .y = y, .result = &result, .n = n, .pool = pool};

    for(size_t i = 0; i < n; i++) {
        x[i] = rand_range_double(1, 200);
        y[i] = rand_range_double(1, 200);
    }

    #define _N 50
    #define _L 10
    printf("Baseline   : ");
    benchmark(bench_dot, (void *)(&arg), _N, _L, 0b01, NULL);
    printf("OPENMP     : ");
    benchmark(bench_openmp, (void *)(&arg), _N, _L, 0b01, NULL);
    printf("PThreadPool: ");
    benchmark(bench_pthreadpool, (void *)(&arg), _N, _L, 0b01, NULL);

    #ifdef ACCURACY 
    printf("\nAccuracy\n");
    bench_dot((void *)(&arg));
    printf("B  : %16.6f\n", *(arg.result));
    bench_openmp((void *)(&arg));
    printf("OMP: %16.6f\n", *(arg.result));
    bench_pthreadpool((void *)(&arg));
    printf("PTP: %16.6f\n", *(arg.result));
    #endif
    pool_destroy(&pool);
}
