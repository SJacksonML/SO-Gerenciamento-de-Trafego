/* Importando clock.h */
#include "clock.h"
/* Importando unistd.h para usar usleep() */
#include <unistd.h>

/* Variável global estática do relógio */
static ClockState clock_state; 

/* Variável de condição do while loop*/
int is_active = 1;

/* Função de inicialização do relógio
* Coloca o tick em um valor padrão 0
* Inicializa o mutex
* Inicializa a variável de condição (cond)
*/
void clock_init(void) {
    clock_state.tick = 0;
    pthread_mutex_init(&clock_state.mutex, NULL);
    pthread_cond_init(&clock_state.cond, NULL);
}

/* Função de destruição do relógio
* Destrói o mutex
* Destrói a variável de condição (cond)
* Coloca o tick em um valor padrão 0
*/
void clock_destroy(void) {
    pthread_mutex_destroy(&clock_state.mutex);
    pthread_cond_destroy(&clock_state.cond);
    clock_state.tick = 0;
}

/* Função de esperar o próximo tick, é usada para espera o tempo avançar
* Tranca o mutex
* Enquanto o tick não mudar, ela esperava o alerta da variável de condição
* Destranca o mutex
*/
void clock_wait_tick(void) {
    pthread_mutex_lock(&clock_state.mutex);
    int initial_tick = clock_state.tick;
    while (initial_tick == clock_state.tick) {
        pthread_cond_wait(&clock_state.cond, &clock_state.mutex);
    }
    pthread_mutex_unlock(&clock_state.mutex);

}

/* Função principal do relógio, é usada para avançar o tempo
* Usada usleep pra evitar busy wait
* Tranca o mutex
* Aumenta o tick em 1
* Usada pthread_cond_broadcast para alertar as threads que o tick mudou
* Destranca o mutex
*/
void *clock_thread(void *arg) {
    while (is_active) {
        usleep(100000);

        pthread_mutex_lock(&clock_state.mutex);
        clock_state.tick += 1;
        pthread_cond_broadcast(&clock_state.cond);
        pthread_mutex_unlock(&clock_state.mutex);
    }
    return NULL;

}

/* Função de parar o relógio
* Coloca is_active em 0 para encerrar o while da clock_thread
*/
void clock_stop(void) {
    is_active = 0;
}

/* Função de receber o tick
* Retorna o tick do clock state
*/
int clock_get_tick(void) {
    return clock_state.tick;
}