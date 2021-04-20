#include "pool.h"

typedef struct worker_cleanup_arg_t {
    queue_t** node;
    pthread_mutex_t* lock;
} worker_cleanup_arg_t;
// cleanup worker when it is canceld
// do this by destroying the node and unlocking the mutex
void worker_cleanup(void* arg) {
    int ret;
    worker_cleanup_arg_t* cleanup_arg = (worker_cleanup_arg_t*)arg;
    // get the data and destroy the node
    if(*(cleanup_arg->node) != NULL)
        queue_node_destroy(cleanup_arg->node, NULL);

    // unlock the queue
    ret = pthread_mutex_unlock(cleanup_arg->lock);
}

void* worker(void* arg) {
    int ret, oldstate;
    pool_t* pool = (pool_t*)arg;
    struct timespec timeout;
    th_calc_timespec(&timeout, 0, 10ull * 1000ull * 1000ull); // 10ms

    while(1) {
        // we can cancel anytime in the aquisition of a new task
        ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
        // check if we are cancled
        pthread_testcancel();

        // lock the queue
        ret = pthread_mutex_lock(&(pool->queue_lock));

        // try and get the next task, if empty wait for a signal
        queue_t* next = NULL;
        task_t* task = NULL;

        // setup cancel handler
        worker_cleanup_arg_t cleanup_arg = {.node = &next,
                                            .lock = &(pool->queue_lock)};
        pthread_cleanup_push(&worker_cleanup, (void*)(&cleanup_arg));

        while((next = queue_dequeue(&(pool->queue))) == NULL) {
            ret = pthread_cond_timedwait(&(pool->queue_signal),
                                         &(pool->queue_lock), &timeout);
        }
        task = (task_t*)(next->data); // get the data out of the node before
                                      // cleanup
        pthread_cleanup_pop(1);       // non-zero value to execute cleanup

        // check if we are cancled
        pthread_testcancel();

        // we have a task, cannot cancel now
        ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

        // we should have a valid task now, complete the task
        // if there is a result, use it, otherwise dont
        if(task->result)
            *(task->result) = task->function(task->argument);
        else
            task->function(task->argument);

        task->done = 1;
        // task is complete, signal
        ret = pthread_cond_signal(&(task->done_signal));
    }

    return NULL;
}

// init a pool and start up the threads
pool_t* pool_init(int num_threads) {
    int ret;
    pool_t* pool = (pool_t*)malloc(sizeof(pool_t));
    pool->num_threads = num_threads;

    // allocate the queue
    pool->queue = queue_init();
    // init mutex, if fail free resources and return NULL
    ret = pthread_mutex_init(&(pool->queue_lock), NULL);
    if(ret != 0) {
        queue_destroy(&(pool->queue), NULL);
        free(pool);
        return NULL;
    }
    // init cond, if fail free resources and return NULL
    ret = pthread_cond_init(&(pool->queue_signal), NULL);
    if(ret != 0) {
        // should never be non zero
        ret = pthread_mutex_destroy(&(pool->queue_lock));
        queue_destroy(&(pool->queue), NULL);
        free(pool);
        return NULL;
    }

    // alloc threads
    pool->threads = (pthread_t*)malloc(pool->num_threads * sizeof(pthread_t));
    // create the threads
    for(unsigned int i = 0; i < pool->num_threads; i++) {
        ret = pthread_create(&(pool->threads[i]), NULL, worker, (void*)pool);
    }

    return pool;
}

void task_destructor(void* arg) {
    if(arg == NULL)
        return;
    task_t* data = (task_t*)arg;
    pool_task_destroy(&data);
}

// destroy and close threads
void pool_destroy(pool_t** pool) {
    int ret;
    // cancel all threads
    for(unsigned int i = 0; i < (*pool)->num_threads; i++) {
        pthread_cancel((*pool)->threads[i]);
    }

    // join the threads
    for(unsigned int i = 0; i < (*pool)->num_threads; i++) {
        pthread_join((*pool)->threads[i], NULL);
    }
    // cleanup the threads array
    free((*pool)->threads);

    // cleanup the mutexs
    ret = pthread_cond_destroy(&((*pool)->queue_signal));
    ret = pthread_mutex_destroy(&((*pool)->queue_lock));

    // destroy remaining tasks
    queue_destroy(&((*pool)->queue), task_destructor);

    // free the pool
    free(*pool);
    pool = NULL;
}

// submit work to pool
task_t* pool_submit(pool_t* pool, void* (*function)(void*), void* argument,
                    void** result) {
    int ret;
    // init basic task items
    task_t* task = (task_t*)malloc(sizeof(task_t));
    task->argument = argument;
    task->result = result;
    task->function = function;
    task->done = 0; // not done

    // init the signal
    ret = pthread_mutex_init(&(task->done_lock), NULL);
    if(ret != 0) {
        free(task);
        return NULL;
    }
    ret = pthread_cond_init(&(task->done_signal), NULL);
    if(ret != 0) {
        // should never be non zero
        ret = pthread_mutex_destroy(&(task->done_lock));
        free(task);
        return NULL;
    }

    // submit task to pool
    // lock the queue
    ret = pthread_mutex_lock(&(pool->queue_lock));
    // init and add
    queue_t* node = queue_node_init();
    node->data = (void*)task;
    queue_enqueue(&(pool->queue), node);
    // unlock
    ret = pthread_mutex_unlock(&(pool->queue_lock));
    // signal we have a new task
    ret = pthread_cond_signal(&(pool->queue_signal));

    return task;
}

// wait for a specific work to complete
void pool_wait(task_t** task) {
    int ret;
    struct timespec timeout;
    th_calc_timespec(&timeout, 0, 10ull * 1000ull * 1000ull); // 10ms
    // while not done, wait for signal
    ret = pthread_mutex_lock(&((*task)->done_lock));
    while(!(*task)->done) {
        ret = pthread_cond_timedwait(&((*task)->done_signal),
                                     &((*task)->done_lock), &timeout);
        DPRINTF("WAIT %d\n", ret);
    }
    ret = pthread_mutex_unlock(&((*task)->done_lock));

    // wait is done, we can destroy the task
    pool_task_destroy(task);
}

// destroy the task
void pool_task_destroy(task_t** task) {
    int ret;
    ret = pthread_cond_destroy(&((*task)->done_signal));
    ret = pthread_mutex_destroy(&((*task)->done_lock));
    free(*task);
    *task = NULL;
}
