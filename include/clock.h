#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>

typedef struct {
    int tick;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ClockState;

ClockState *clock_init(void);

void clock_destroy(ClockState *clock_state);

void clock_wait_tick();

void *clock_thread(void *arg);

int clock_get_tick();

#endif