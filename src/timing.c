#include "timing.h"
#include "config.h"
#include <time.h>

// Calcula diferença de tempo em nanosegundos
long long timespec_diff_ns(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000000000LL + (end->tv_nsec - start->tv_nsec);
}

void timing_wait_for_interval(struct timespec *loop_start) {
    struct timespec loop_end;
    
    // Marca o fim do loop e calcula o tempo decorrido
    clock_gettime(CLOCK_MONOTONIC, &loop_end);
    long long elapsed_ns = timespec_diff_ns(loop_start, &loop_end);
    
    // Calcula o tempo restante para atingir o intervalo
    long long remaining_ns = TARGET_INTERVAL_NS - elapsed_ns;
    
    // Se ainda há tempo restante, aguarda
    if (remaining_ns > 0) {
        struct timespec sleep_time;
        sleep_time.tv_sec = remaining_ns / 1000000000LL;
        sleep_time.tv_nsec = remaining_ns % 1000000000LL;
        nanosleep(&sleep_time, NULL);
    }
}