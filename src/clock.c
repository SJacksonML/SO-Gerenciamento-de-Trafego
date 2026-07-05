#include "clock.h"

static ClockState clock_state;

void clock_init(void) {
    clock_state.tick = 0;
    pthread_mutex_init(&clock_state.mutex, NULL);
    pthread_cond_init(&clock_state.cond, NULL);
}

void clock_destroy(void) {
    pthread_mutex_destroy(&clock_state.mutex);
    pthread_cond_destroy(&clock_state.cond);
    clock_state.tick = 0;
}

void clock_wait_tick(void) {
    pthread_mutex_lock(&clock_state.mutex);
    int initial_tick = clock_state.tick;
    while (initial_tick == clock_state.tick) {
        pthread_cond_wait(&clock_state.cond, &clock_state.mutex);
    }
    pthread_mutex_unlock(&clock_state.mutex);

}

int clock_get_tick(void) {
    return clock_state.tick;
}