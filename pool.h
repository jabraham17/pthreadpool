
#ifndef _POOL_H_
#define _POOL_H_

#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include "dprintf.h"

struct pool_t {
    //thread management
    unsigned int num_threads;
    pthread_t *threads;

    //queue management
    struct queue_t* queue;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_signal;
};

struct task_t {
    void* argument;
    void* (*function)(void*);
    void** result;
    pthread_cond_t done_signal;
    pthread_mutex_t done_lock;
};

//init a pool and start up the threads
struct pool_t* pool_init(int num_threads);
//destroy and close threads
void pool_destroy(struct pool_t** pool);
//submit work to pool
struct task_t* pool_submit(struct pool_t* pool, void* (*function)(void*), void* argument, void** result);
//wait for a specific work to complete
void pool_wait(struct task_t* task);



#endif