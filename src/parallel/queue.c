
#include "queue.h"

#include "log.h"

void queue_push(queue_t *queue, void *value)
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == queue->capacity)
    {
        pthread_cond_wait(&(queue->cond_full), &(queue->mutex));
    }

#ifdef TRACE
    log_info("Enqueue a new set");
#endif

    queue->buffer[queue->in] = value;
    ++ queue->size;
    ++ queue->in;
    queue->in %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_empty));
}

void *queue_pop(queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 0)
    {
        pthread_cond_wait(&(queue->cond_empty), &(queue->mutex));
    }
    void *value = queue->buffer[queue->out];

#ifdef TRACE
    log_info("Dequeued a set");
#endif

    -- queue->size;
    ++ queue->out;
    queue->out %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_full));
    return value;
}

int queue_size(queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    int size = queue->size;
    pthread_mutex_unlock(&(queue->mutex));
    return size;
}
