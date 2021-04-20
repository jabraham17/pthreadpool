# pthreadpool

This is my implementation of a thread pool using POSIX threads, also known as pthreads.

## Use

A pool is created by calling `pool_t* pool_init(int num_threads)`, which will allocate all the necessary items. From there, tasks can be submitted using `task_t* pool_submit(pool_t* pool, void* (*function)(void*), void* argument, void** result);`. This will be executed as `*result = function(argument)`. If `result` is `NULL`, it will be `function(argument)`. `pool_submit` will append to the task queue, there is no guarantees on when it will be executed. Calling `void pool_wait(task_t** task)` will block the calling thread until the task has finished executing. It will also free the task, if this function is not used this will cause a memory leak. You can manually free a task with `void pool_task_destroy(task_t** task)`. When you are finished with the pool, free the resources with `void pool_destroy(pool_t** pool)`.

## Implementation Overview

When a new pool is created, it allocates a certain amount of threads and a worker queue. Users submit work to the queue, and the threads take the work from the queue, execute it, then wait for more work.

### Queue Implementation

My queue implementation is a very basic doubly linked queue that just supports two operations, adding items to the head and removing from the tail. Since it is a doubly linked queue, only one pointer needs to be kept. The head's `next` is the next item to come out of the queue and the head's `prev` is the last item added.

I implemented my queue with a dummy head node. So an empty queue is signified by a single empty node that points to itself, rather than just a empty pointer. This solves many of the special cases that come about in linked list based data structures, making the code easier to maintain and read, at the cost of some transparency. This method makes it slightly harder for users to write ad-hoc functions using the queue, they pretty much have to either use my existing functions or be aware of the dummy node and code accordingly.

### Pool Implementation

The pool data structure contains a list of the threads it has available to it and a queue to hold the work. Each of the threads is running a worker thread which uses the following algorithm.

```
worker:
    while true:
        check for new tasks
        if no new tasks, sleep

        if there is a task, execute it to completion
        signal task completion
```

While the worker is waiting for a new task, it can be canceled and closed down by the pool. However, while it is actively executing a task it cannot be interrupted.
