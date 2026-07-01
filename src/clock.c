#include "clock.h"

ClockState clock_state;

ClockState *clock_init(void) {
    ClockState *new_clock_state = malloc(sizeof(ClockState));
    new_clock_state->tick = 0;
    pthread_mutex_init(&new_clock_state->mutex, NULL);
    pthread_cond_init(&new_clock_state->cond, NULL);
    return new_clock_state;
}

void clock_destroy(ClockState *clock_state) {
    pthread_mutex_destroy(&clock_state->mutex);
    pthread_cond_destroy(&clock_state->cond);
    clock_state->tick = 0;
}