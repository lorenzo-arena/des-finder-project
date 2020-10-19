#include "stopwatch.h"

static struct timespec start = { 0, 0 };
static struct timespec end = { 0, 0 };
static struct timespec delta = { 0, 0 };

#define NS_PER_SECOND 1000000000

void stopwatch_start()
{
    clock_gettime(CLOCK_MONOTONIC, &start);
}

void stopwatch_stop()
{
    clock_gettime(CLOCK_MONOTONIC, &end);

    delta.tv_nsec = end.tv_nsec - start.tv_nsec;
    delta.tv_sec  = end.tv_sec - start.tv_sec;
    if (delta.tv_sec > 0 && delta.tv_nsec < 0)
    {
        delta.tv_nsec += NS_PER_SECOND;
        delta.tv_sec--;
    }
    else if (delta.tv_sec < 0 && delta.tv_nsec > 0)
    {
        delta.tv_nsec -= NS_PER_SECOND;
        delta.tv_sec++;
    }
}

struct timespec stopwatch_get_elapsed()
{
    return delta;
}
