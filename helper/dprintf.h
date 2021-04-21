
#ifndef _DPRINTF_H_
#define _DPRINTF_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define DPRINTF(fmt, ...)                                                      \
    printf("DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,            \
           ##__VA_ARGS__)
#else
#define DPRINTF(fmt, ...)
#endif

#define EPRINTF(__FMT, ...)                                                    \
    fprintf(stderr, "ERROR: %s:%d:%s: " __FMT, __FILE__, __LINE__, __func__,   \
            ##__VA_ARGS__)

#define CHECK_RETURN(__RETVAL, __MSG_FMT, ...)                                 \
    if((__RETVAL) != 0) {                                                      \
        EPRINTF("%d: " __MSG_FMT "\n", __RETVAL, ##__VA_ARGS__);               \
    }
#define CHECK_EXPR(__EXPR, __MSG_FMT, ...)                                     \
    if(!(__EXPR)) {                                                            \
        EPRINTF("CHECK " #__EXPR ": " __MSG_FMT "\n", ##__VA_ARGS__);          \
    }
#define ASSERT(__EXPR, __MSG_FMT, ...)                                         \
    if(!(__EXPR)) {                                                            \
        fprintf(stderr,                                                        \
                "ASSERT: %s:%d:%s:" #__EXPR ": "__MSG_FMT                      \
                "\n",                                                          \
                __FILE__, __LINE__, __func__, ##__VA_ARGS__);                  \
        exit(1);                                                               \
    }

#endif
