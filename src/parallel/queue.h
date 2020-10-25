#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>

typedef struct queue
{
    void **buffer;
    const int capacity;
    int size;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
} queue_t;

void queue_push(queue_t *queue, void *value);
void *queue_pop(queue_t *queue);
int queue_size(queue_t *queue);

#endif
