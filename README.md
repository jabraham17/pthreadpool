# pthreadpool

This is my implementation of a thread pool using POSIX threads, also known as pthreads.

## Use

A pool is created by calling `pool_t* pool_init(int num_threads)`, which will allocate all the necessary items. From there, tasks can be submitted using `task_t* pool_submit(pool_t* pool, void* (*function)(void*), void* argument, void** result);`. This will be executed as `*result = function(argument)`. If `result` is `NULL`, it will be `function(argument)`. `pool_submit` will append to the task queue, there is no guarantees on when it will be executed. Calling `void pool_wait(task_t** task)` will block the calling thread until the task has finished executing. It will also free the task, if this function is not used this will cause a memory leak. You can manually free a task with `void pool_task_destroy(task_t** task)`. When you are finished with the pool, free the resources with `void pool_destroy(pool_t** pool)`.

Running `make` at the root (or inside the `pthreadpool` directory) will built a static library `libptp.a` that you can link against and use in applications.

## Benchmark

The benchmark I developed for this is a dot product using 3 methods of parallel code: hand coded SIMD, OpenMP, and my pthreadpool implementation. The benchmark can be run as `./src/bench/bench.sh`. Below are the results from my computer.

```
1 Threads
Baseline   : Mean: 112.5863ms Variance:   3.4704ms
Vector     : Mean:  68.5816ms Variance:   0.4813ms
OPENMP     : Mean: 114.0896ms Variance:   1.3639ms
PThreadPool: Mean: 130.2516ms Variance: 423.3991ms

2 Threads
Baseline   : Mean: 120.2690ms Variance: 102.6837ms
Vector     : Mean:  73.2141ms Variance:  40.8585ms
OPENMP     : Mean:  69.8895ms Variance: 112.3846ms
PThreadPool: Mean:  64.3326ms Variance:  43.4688ms

4 Threads
Baseline   : Mean: 129.1516ms Variance: 238.1945ms
Vector     : Mean:  77.9944ms Variance:  97.4522ms
OPENMP     : Mean:  52.3732ms Variance:  14.3590ms
PThreadPool: Mean:  52.1564ms Variance:   7.0968ms

8 Threads
Baseline   : Mean: 112.5771ms Variance:   1.6184ms
Vector     : Mean:  63.1182ms Variance:   0.3399ms
OPENMP     : Mean:  50.0403ms Variance:   0.8563ms
PThreadPool: Mean:  51.3388ms Variance:   1.9325ms

16 Threads
Baseline   : Mean: 112.2270ms Variance:   1.4769ms
Vector     : Mean:  67.1469ms Variance:   0.3271ms
OPENMP     : Mean:  50.3816ms Variance:  16.1614ms
PThreadPool: Mean:  50.8659ms Variance:   1.3040ms
```

As you can see, my pthreadpool implementation stays pretty much comparable to OpemMP.

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
