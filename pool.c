#include "pool.h"

void *worker(void *arg) {
    int ret;
    struct pool_t *pool = (struct pool_t*)arg;

    // TODO: set cancel state
    // TODO: handle ret vals
    while(1) {
        //DPRINTF("In worker %p\n", pthread_self());
        // lock the queue
        ret = pthread_mutex_lock(&(pool->queue_lock));

        // try and get the next task, if empty wait for a signal
        struct queue_t* next = NULL;
        struct task_t* task = NULL;
        while((next = queue_dequeue(&(pool->queue))) == NULL) {
            ret = pthread_cond_wait(&(pool->queue_signal), &(pool->queue_lock));
            //DPRINTF("I woke up %p\n", pthread_self());
        }

        //get the data and destroy the node
        task = (struct task_t*)(next->data);
        //queue_node_destroy(&(pool->queue));

        //unlock the queue
        ret = pthread_mutex_unlock(&(pool->queue_lock));

        //we should have a valid task now, complete the task
        //if there is a result, use it, otherwise dont
        if(task->result) *(task->result) = task->function(task->argument);
        else task->function(task->argument);

        //task is complete, signal
        ret = pthread_cond_signal(&(task->done_signal));
        //TODO: maybe now we can destroy the task, or maybe we should do this in pool_wait
        
    }

    return NULL;
}

// init a pool and start up the threads
struct pool_t *pool_init(int num_threads) {
    int ret;
    struct pool_t *pool = (struct pool_t *)malloc(sizeof(struct pool_t));
    pool->num_threads = num_threads;

    // allocate the queue
    pool->queue = queue_init();
    // init mutex, if fail free resources and return NULL
    ret = pthread_mutex_init(&(pool->queue_lock), NULL);
    if(ret != 0) {
        queue_destroy(&(pool->queue));
        free(pool);
        return NULL;
    }
    // init cond, if fail free resources and return NULL
    ret = pthread_cond_init(&(pool->queue_signal), NULL);
    if(ret != 0) {
        // should never be non zero
        ret = pthread_mutex_destroy(&(pool->queue_lock));
        queue_destroy(&(pool->queue));
        free(pool);
        return NULL;
    }

    // alloc threads
    pool->threads = (pthread_t *)malloc(pool->num_threads * sizeof(pthread_t));
    // create the threads
    for(unsigned int i = 0; i < pool->num_threads; i++) {
        ret = pthread_create(&(pool->threads[i]), NULL, worker, (void *)pool);
        /*if(ret != 0) {
            //should never be non zero
            ret = pthread_mutex_destroy(&(pool->queue_lock));
            queue_destroy(&(pool->queue));
            free(pool);
            return NULL;
        }*/
    }

    return pool;
}

// destroy and close threads
void pool_destroy(struct pool_t **pool) {
    for(unsigned int i = 0; i < (*pool)->num_threads; i++) {
        pthread_join((*pool)->threads[i], NULL);
    }

}

// submit work to pool
struct task_t *pool_submit(struct pool_t *pool, void *(*function)(void *),
                           void *argument, void **result) {
    int ret;
    // init basic task items
    struct task_t *task = (struct task_t *)malloc(sizeof(struct task_t));
    task->argument = argument;
    task->result = result;
    task->function = function;

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

    // TODO: handle rets

    //submit task to pool
    //lock the queue
    ret = pthread_mutex_lock(&(pool->queue_lock));
    //init and add
    struct queue_t* node = queue_node_init();
    node->data = (void*)task;
    queue_enqueue(&(pool->queue), node);
    //unlock
    ret = pthread_mutex_unlock(&(pool->queue_lock));
    //signal we have a new task
    ret = pthread_cond_signal(&(pool->queue_signal));


    return task;
}

// wait for a specific work to complete
void pool_wait(struct task_t *task) {}