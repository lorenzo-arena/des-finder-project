#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <time.h>

void stopwatch_start();
void stopwatch_stop();
struct timespec stopwatch_get_elapsed();

#endif
