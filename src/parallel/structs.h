#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <pthread.h>

#include "common.h"

#define PWD_SET_DIMENSION 300

typedef struct thread_arg
{
    char pwd_set[PWD_SET_DIMENSION][PWD_DIMENSION + 1];
    const char *hash;
    const char *salt;
} thread_arg_t;

typedef struct result
{
    volatile bool found;
    volatile bool ended;
    char password[PWD_DIMENSION + 1];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} result_t;

typedef struct producer_args {
    const char *filename;
    const char *hash;
    const char *salt;
    queue_t *queue;
} producer_args_t;

#endif
