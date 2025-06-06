#ifndef TIMING_H
#define TIMING_H

#include <time.h>

long long timespec_diff_ns(struct timespec *start, struct timespec *end);

void timing_wait_for_interval(struct timespec *loop_start);

#endif // TIMING_H