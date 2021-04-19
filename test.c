
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
    int id = (unsigned long long )arg / CONST;
    unsigned long pid = ((unsigned long)pthread_self() & 0xFFFFFFFF) >> 12;
    printf("%10llu: Start  task %3d in 0x%lx\n", th_gettime(), id, pid);
    for(unsigned long long i = 0; i < (unsigned long long )arg; i++)  {
        asm ("nop");
    }
    printf("%10llu: Finish task %3d in 0x%lx\n", th_gettime(), id, pid);
    return NULL;
}

int main() {

    //dummy_print((void*)100000);

    //dummy_print((void*)100000);

    struct pool_t* pool = pool_init(5);

    for(unsigned int i = 0; i < 20; i++) {
        struct task_t* t = pool_submit(pool, dummy_print, (void*)((i+1)*CONST), NULL);
        //DPRINTF("task %p\n", t);
    }

        for(unsigned long long i = 0; i < 1000000ull; i++)  {
        asm ("nop");
    }

    
    pool_destroy(&pool);
    

    return 0;
}