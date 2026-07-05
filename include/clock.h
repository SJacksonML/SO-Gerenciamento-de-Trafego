#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>

typedef struct {
    int tick;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ClockState;

void clock_init(void);

void clock_destroy(void);

void clock_wait_tick(void);

void *clock_thread(void *arg);

int clock_get_tick(void);

#endif