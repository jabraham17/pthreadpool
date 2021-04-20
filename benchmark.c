
// benchmark using this to optimize a loop

#include <immintrin.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "pool.h"
#include "timespec_helper.h"
#include "timing.h"

// baseline kernel
double dot(double* x, double* y, size_t n) {
    size_t i = 0;
    double sum = 0;
#ifdef VECTORIZE
    __m256d temp1, temp2, temp3, temp4;
    temp1 = temp2 = temp3 = temp4 = _mm256_set1_pd(0);
    __m256d sum12, sum34, vsum, shuf;
    for(i = 0; i < n - 16; i += 16) {
        temp1 = _mm256_fmadd_pd(_mm256_load_pd(x + i), _mm256_load_pd(y + i),
                                temp1);
        temp2 = _mm256_fmadd_pd(_mm256_load_pd(x + i + 4),
                                _mm256_load_pd(y + i + 4), temp2);
        temp3 = _mm256_fmadd_pd(_mm256_load_pd(x + i + 8),
                                _mm256_load_pd(y + i + 8), temp3);
        temp4 = _mm256_fmadd_pd(_mm256_load_pd(x + i + 12),
                                _mm256_load_pd(y + i + 12), temp4);
    }
    sum12 = _mm256_add_pd(temp1, temp2);
    sum34 = _mm256_add_pd(temp3, temp4);
    vsum = _mm256_add_pd(sum12, sum34);

    // A  B  C  D
    // B  A  D  C
    // AB BA CD DC
    // CD DC AB BA
    shuf = _mm256_permute_pd(vsum, 0b00000101);
    vsum = _mm256_add_pd(vsum, shuf);
    shuf = _mm256_permute2f128_pd(vsum, vsum, 0b00100001);
    vsum = _mm256_add_pd(vsum, shuf); // even though single, do it anyways

    sum = _mm_cvtsd_f64(_mm256_extractf128_pd(vsum, 0));
#endif
    for(/*nothing*/; i < n; i++) {
        sum += x[i] * y[i];
    }
    return sum;
}

#ifndef NUM_THREADS
#define NUM_THREADS 1
#endif

// openmp implementation
double dot_openmp(double* x, double* y, size_t n) {
    double sum = 0;
    size_t i = 0;

#ifdef VECTORIZE
    __m256d temp1, temp2, temp3, temp4;
    temp1 = temp2 = temp3 = temp4 = _mm256_set1_pd(0);
    __m256d sum12, sum34, vsum, shuf;

#endif

#ifdef VECTORIZE
#pragma omp parallel private(i, temp1, temp2, temp3, temp4, sum12, sum34,      \
                             vsum, shuf) num_threads(NUM_THREADS)
#else
#pragma omp parallel private(i) num_threads(NUM_THREADS)
#endif
    {
#ifdef VECTORIZE
#pragma omp for
        for(i = 0; i < n - 16; i += 16) {
            temp1 = _mm256_fmadd_pd(_mm256_load_pd(x + i),
                                    _mm256_load_pd(y + i), temp1);
            temp2 = _mm256_fmadd_pd(_mm256_load_pd(x + i + 4),
                                    _mm256_load_pd(y + i + 4), temp2);
            temp3 = _mm256_fmadd_pd(_mm256_load_pd(x + i + 8),
                                    _mm256_load_pd(y + i + 8), temp3);
            temp4 = _mm256_fmadd_pd(_mm256_load_pd(x + i + 12),
                                    _mm256_load_pd(y + i + 12), temp4);
        }
        sum12 = _mm256_add_pd(temp1, temp2);
        sum34 = _mm256_add_pd(temp3, temp4);
        vsum = _mm256_add_pd(sum12, sum34);

        // A  B  C  D
        // B  A  D  C
        // AB BA CD DC
        // CD DC AB BA
        shuf = _mm256_permute_pd(vsum, 0b00000101);
        vsum = _mm256_add_pd(vsum, shuf);
        shuf = _mm256_permute2f128_pd(vsum, vsum, 0b00100001);
        vsum = _mm256_add_pd(vsum, shuf); // even though single, do it anyways

        sum = _mm_cvtsd_f64(_mm256_extractf128_pd(vsum, 0));
#endif
        for(/*nothing*/; i < n; i++) {
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
    size_t i = parg->low;

    #ifdef VECTORIZE
    __m256d temp1, temp2, temp3, temp4;
    temp1 = temp2 = temp3 = temp4 = _mm256_set1_pd(0);
    __m256d sum12, sum34, vsum, shuf;
    for(/*nothing*/; i < parg->high - 16; i += 16) {
        temp1 = _mm256_fmadd_pd(_mm256_load_pd(parg->x + i), _mm256_load_pd(parg->y + i),
                                temp1);
        temp2 = _mm256_fmadd_pd(_mm256_load_pd(parg->x + i + 4),
                                _mm256_load_pd(parg->y + i + 4), temp2);
        temp3 = _mm256_fmadd_pd(_mm256_load_pd(parg->x + i + 8),
                                _mm256_load_pd(parg->y + i + 8), temp3);
        temp4 = _mm256_fmadd_pd(_mm256_load_pd(parg->x + i + 12),
                                _mm256_load_pd(parg->y + i + 12), temp4);
    }
    sum12 = _mm256_add_pd(temp1, temp2);
    sum34 = _mm256_add_pd(temp3, temp4);
    vsum = _mm256_add_pd(sum12, sum34);

    // A  B  C  D
    // B  A  D  C
    // AB BA CD DC
    // CD DC AB BA
    shuf = _mm256_permute_pd(vsum, 0b00000101);
    vsum = _mm256_add_pd(vsum, shuf);
    shuf = _mm256_permute2f128_pd(vsum, vsum, 0b00100001);
    vsum = _mm256_add_pd(vsum, shuf); // even though single, do it anyways

    sum = _mm_cvtsd_f64(_mm256_extractf128_pd(vsum, 0));
#endif
    for(/*nothing*/; i < parg->high; i++) {
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
        threads[i] =
            pool_submit(pool, dot_pthreadpool_worker, (void*)(args + i), NULL);
    }

    double sum = 0;
    for(int i = 0; i < NUM_THREADS; i++) {
        pool_wait(&threads[i]);
        sum += sums[i];
    }
    return sum;
}

typedef struct {
    double* x;
    double* y;
    double* result;
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

int main(__attribute((unused)) int argc, __attribute((unused)) char** argv) {

    srand(time(NULL));

    size_t n = 1024 * 1024 * 8;

    double result;
    double* x = (double*)aligned_alloc(64, n * sizeof(double));
    double* y = (double*)aligned_alloc(64, n * sizeof(double));

    pool_t* pool = pool_init(NUM_THREADS);

    test_args_t arg = {.x = x, .y = y, .result = &result, .n = n, .pool = pool};

    for(size_t i = 0; i < n; i++) {
        x[i] = rand_range_double(1, 200);
        y[i] = rand_range_double(1, 200);
    }

#define _N 50
#define _L 10
    printf("Baseline   : ");
    benchmark(bench_dot, (void*)(&arg), _N, _L, 0b01, NULL);
    printf("OPENMP     : ");
    benchmark(bench_openmp, (void*)(&arg), _N, _L, 0b01, NULL);
    printf("PThreadPool: ");
    benchmark(bench_pthreadpool, (void*)(&arg), _N, _L, 0b01, NULL);

#ifdef ACCURACY
    printf("\nAccuracy\n");
    bench_dot((void*)(&arg));
    printf("B  : %16.6f\n", *(arg.result));
    bench_openmp((void*)(&arg));
    printf("OMP: %16.6f\n", *(arg.result));
    bench_pthreadpool((void*)(&arg));
    printf("PTP: %16.6f\n", *(arg.result));
#endif
    pool_destroy(&pool);
}
