#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "processor.h"
#include "queue.h"
#include "common.h"
#include "log.h"
#include "structs.h"

#define THREAD_NUM 8

void *data[THREAD_NUM];

result_t result = {
    .found = false,
    .ended = false,
    .password = { 0 },
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER
};

queue_t queue = {
    .buffer = data,
    .capacity = sizeof(data)/sizeof(data[0]),
    .size = 0,
    .in = 0,
    .out = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond_full = PTHREAD_COND_INITIALIZER,
    .cond_empty = PTHREAD_COND_INITIALIZER
};

static void unlock_result_mutex(__attribute__ ((unused)) void *arg)
{
    __attribute__ ((unused)) int res = pthread_mutex_unlock(&result.mutex);

#ifdef TRACE
    log_info("Unlock cleanup result: %d", res);
#endif
}

static void unlock_queue_mutex(__attribute__ ((unused)) void *arg)
{
    __attribute__ ((unused)) int res = pthread_mutex_unlock(&queue.mutex);

#ifdef TRACE
    log_info("Unlock cleanup result: %d", res);
#endif
}

static void close_file(void *arg)
{
    if(arg != NULL)
    {
        fclose((FILE *)arg);
    }
}

void *test_pwd_set(void *queue)
{
    // Deferred is the default cancel type, but better be explicit
    int old_state = 0;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_state);

#ifdef TRACE
    pthread_t self;
    self = pthread_self();
    log_info("Starting thread with ID: %lu", self);
#endif

    // Push the cleanup routines to unlock mutexes
    pthread_cleanup_push(unlock_result_mutex, NULL);
    pthread_cleanup_push(unlock_queue_mutex, NULL);

    // TODO : check always the result with the mutex
    pthread_mutex_lock(&result.mutex);
    while(!result.found)
    {
        pthread_mutex_unlock(&result.mutex);
        thread_arg_t *arg = (thread_arg_t *)queue_pop((queue_t *)queue);

        for(int pwd_idx = 0; pwd_idx < PWD_SET_DIMENSION; pwd_idx++)
        {
#ifdef TRACE
            //log_info("Processing pwd: %s", arg->pwd_set[pwd_idx]);
#endif

            if(test_password(arg->pwd_set[pwd_idx], arg->hash, arg->salt))
            {
                pthread_mutex_lock(&result.mutex);
                result.found = true;
                memcpy(result.password, arg->pwd_set[pwd_idx], PWD_DIMENSION);
                pthread_mutex_unlock(&result.mutex);
                pthread_cond_broadcast(&result.cond);

                free(arg);
                return NULL;
            }
        }

        free(arg);

        pthread_mutex_lock(&result.mutex);
    }

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    return NULL;
}

void *start_processing(void *args)
{
    // Deferred is the default cancel type, but better be explicit
    int old_state = 0;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_state);

#ifdef TRACE
    pthread_t self;
    self = pthread_self();
    log_info("Starting thread with ID: %lu", self);
#endif

    FILE *file = NULL;
    int pwd_idx = 0;
    ssize_t read = 0;
    size_t line_len = 0;
    const producer_args_t *producer_args = (const producer_args_t *)args;

    file = fopen(producer_args->filename, "r");

    // Push the cleanup routines to unlock mutexes
    pthread_cleanup_push(unlock_result_mutex, NULL);
    pthread_cleanup_push(unlock_queue_mutex, NULL);

    // The close file cleanup routine must be pushed after the opening
    pthread_cleanup_push(close_file, file);

    if(file == NULL)
    {
        log_error("Could't open file: %s", producer_args->filename);
    }
    else
    {
        // TODO : get the mutex to check the result!
        pthread_mutex_lock(&result.mutex);
        while(!result.found && (read != -1))
        {
            pthread_mutex_unlock(&result.mutex);
            char *line = NULL;

            thread_arg_t *arg = calloc(sizeof(thread_arg_t), 1);
            arg->hash = producer_args->hash;
            arg->salt = producer_args->salt;

            assert(arg != NULL);

            while(((read = getline(&line, &line_len, file)) != -1) && (pwd_idx < PWD_SET_DIMENSION))
            {
                // I need to check the string length as getline seems
                // to allocate at least 120 bytes by default even for
                // line containing just '\n'
                if(strnlen(line, line_len) >= PWD_DIMENSION)
                {
                    // Fill the set
                    memcpy(arg->pwd_set[pwd_idx], line, PWD_DIMENSION);
                    arg->pwd_set[pwd_idx][PWD_DIMENSION] = '\0';
                    pwd_idx++;
                }
            }

            if(line)
            {
                free(line);
            }

            if(pwd_idx > 0)
            {
                pwd_idx = 0;
                queue_push(&queue, arg);
            }

            pthread_mutex_lock(&result.mutex);
        }
    }

    pthread_mutex_lock(&result.mutex);
    result.ended = true;
    pthread_mutex_unlock(&result.mutex);

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cond_broadcast(&result.cond);
    return NULL;
}

int process_file(const char *filename, const char *hash, const char *salt)
{
    producer_args_t producer_args = {
        .filename = filename,
        .hash = hash,
        .salt = salt
    };
    pthread_t producer;
    pthread_t consumers[THREAD_NUM];

    pthread_mutexattr_t result_mutex_att;
    pthread_mutexattr_t queue_mutex_att;

    pthread_mutexattr_init(&result_mutex_att);
    pthread_mutexattr_settype(&result_mutex_att, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&result.mutex, &result_mutex_att);

    pthread_mutexattr_init(&queue_mutex_att);
    pthread_mutexattr_settype(&queue_mutex_att, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&queue.mutex, &queue_mutex_att);

    pthread_create(&producer, NULL, start_processing, &producer_args);

    for(int consumers_idx = 0; consumers_idx < THREAD_NUM; consumers_idx++)
    {
        pthread_create(&consumers[consumers_idx], NULL, test_pwd_set, &queue);
    }

    // This wait is used to check if the whole dictionary has been processed
    // or if the pwd has been found before
    pthread_mutex_lock(&result.mutex);
    while(!result.found && !result.ended)
    {
        pthread_cond_wait(&result.cond, &result.mutex);
    }
    pthread_mutex_unlock(&result.mutex);

    // Wait for the remaining threads
    pthread_cancel(producer);
    pthread_join(producer, NULL);

    for(int consumers_idx = 0; consumers_idx < THREAD_NUM; consumers_idx++)
    {
        pthread_cancel(consumers[consumers_idx]);
        pthread_join(consumers[consumers_idx], NULL);
    }

    if(result.found)
    {
        log_info("Password found: %s", result.password);
    }
    else
    {
        log_error("Password not found!");
        return -1;
    }

    return 0;
}
