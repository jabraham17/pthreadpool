
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "pool.h"
#include "timespec_helper.h"

//#define gettid() syscall(SYS_gettid)

#define CONST 10000000ull

void* dummy_print(void* arg) {
    unsigned long long id = (unsigned long long )arg / CONST;
    unsigned long pid = ((unsigned long)pthread_self() & 0xFFFFFFFF) >> 12;

    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("%14.2f: Start  task %3llu in 0x%lx\n", th_conv_timespec_ms(&t), id, pid);
    for(unsigned long long i = 0; i < (unsigned long long )arg; i++)  {
        __asm__ ("nop");
    }
    struct timespec t2;
    clock_gettime(CLOCK_MONOTONIC, &t2);
    th_sub_timespec(&t, &t2, &t);
    printf("%14.2f: Finish task %3llu in 0x%lx with time %8.2f\n", th_conv_timespec_ms(&t2), id, pid, th_conv_timespec_ms(&t));
    return NULL;
}

int main() {


    struct pool_t* pool = pool_init(5);
    struct task_t* tasks[20];

    for(unsigned int i = 0; i < 20; i++) {
        tasks[i] = pool_submit(pool, dummy_print, (void*)((i+1)*CONST), NULL);
        //pool_wait(t);
        //DPRINTF("task %p\n", t);
    }
    for(int i = 19; i >= 0; i--){
        pool_wait(tasks[i]);
    }

    /*for(unsigned long long i = 0; i < CONST*200ull; i++)  {
        asm ("nop");
    }*/
    struct task_t* t = pool_submit(pool, dummy_print, (void*)(21*CONST), NULL);
    pool_wait(t);

    
    pool_destroy(&pool);
    

    return 0;
}
