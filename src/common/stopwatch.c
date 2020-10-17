#include <time.h>

#include "stopwatch.h"

static struct timespec start = { 0, 0 };
static struct timespec end = { 0, 0 };

static uint32_t elapsed_nanosec = 0;


void stopwatch_start()
{
    elapsed_nanosec = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
}

void stopwatch_stop()
{
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_nanosec = end.tv_nsec - start.tv_nsec;
}

uint32_t stopwatch_get_elapsed_nanosec()
{
    return elapsed_nanosec;
}
