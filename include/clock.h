/* Módulo clock: O relógio global 
* - Sincroniza o tempo
* - Veículos se movem a cada tick
* - O mapa é renderizado a cada tick
*/

#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>

/* Struct do relógio
* tick -> medida de tempo usada
* mutex -> o próprio mutex
* cond -> a variável de condição
*/
typedef struct {
    int tick;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ClockState;

/* Assinaturas das funções */
void clock_init(void);

void clock_destroy(void);

void clock_wait_tick(void);

void *clock_thread(void *arg);

void clock_stop(void);

int clock_get_tick(void);

#endif