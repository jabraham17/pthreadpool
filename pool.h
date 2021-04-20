
#ifndef _POOL_H_
#define _POOL_H_

#include "dprintf.h"
#include "queue.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct pool_t {
    // thread management
    unsigned int num_threads;
    pthread_t* threads;

    // queue management
    queue_t* queue;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_signal;
} pool_t;

typedef struct task_t {
    void* argument;
    void* (*function)(void*);
    void** result;
    unsigned int done;
    pthread_cond_t done_signal;
    pthread_mutex_t done_lock;
} task_t;

// init a pool and start up the threads
pool_t* pool_init(int num_threads);
// destroy and close threads
void pool_destroy(pool_t** pool);
// submit work to pool
task_t* pool_submit(pool_t* pool, void* (*function)(void*), void* argument,
                    void** result);
// wait for a specific work to complete
void pool_wait(task_t** task);
// destroy the task
void pool_task_destroy(task_t** task);

#endif
