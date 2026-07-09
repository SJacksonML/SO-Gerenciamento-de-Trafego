#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>
#include "map.h"

#define N_INTERSECTIONS MAX_INTERSECTIONS

/*
 * Módulo responsável pela sincronização do simulador.
 *
 * Este módulo controla:
 * - exclusão mútua das células do mapa;
 * - sincronização dos cruzamentos com semáforos;
 * - comunicação entre as threads dos veículos.
 *
 * As demais partes do sistema acessam esses recursos apenas
 * por meio das funções declaradas neste arquivo.
 */

/*
 * Fluxo geral da sincronização:
 *
 * 1. Cada veículo solicita a ocupação da próxima célula.
 * 2. O mutex da célula garante exclusão mútua.
 * 3. Ao chegar a um cruzamento, o veículo consulta o semáforo correspondente.
 * 4. Caso o sinal esteja fechado, a thread aguarda em uma variável de condição.
 * 5. Quando o semáforo muda de estado, as threads bloqueadas são notificadas para continuar.
 */

/* ------------- = ------------- = ------------- = ------------- = ------------- = ------------- */

/*
 * Estados possíveis de um semáforo.
 *
 * Cada direção de um cruzamento pode estar liberada (SIGNAL_GREEN) ou bloqueada (SIGNAL_RED).
 */
typedef enum
{
    SIGNAL_RED = 0,
    SIGNAL_GREEN
} SignalState;

/*
 * Representa um cruzamento controlado por semáforo.
 *
 * Cada cruzamento possui sua própria sincronização, permitindo que múltiplos veículos aguardem o sinal.
 *
 * Além da posição do cruzamento no mapa, a estrutura mantém o estado atual do semáforo para cada direção,
 * em vez do estado de cada célula.
 */
 typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    SignalState state[DIR_COUNT];
    
    int row;
    int col;

    int priority_requested;
    Direction priority_dir;
} TrafficSignal;

typedef struct
{
    /* Um mutex para cada célula do mapa */
    pthread_mutex_t cell_mutex[MAX_ROWS][MAX_COLS];

    /* Todos os semáforos do mapa */
    TrafficSignal signals[N_INTERSECTIONS];

    /*
     * Mapeia cada posição do mapa para um índice do vetor signals[].
     *
     * -1 -> a célula não possui semáforo
     *  0 -> signals[0]
     *  1 -> signals[1]
     *  ...
     */
    int signal_index[MAX_ROWS][MAX_COLS];

    /* Quantidade total de semáforos inicializados */
    int num_signals;
} SyncContext;

/*
 * Contexto global.
 */
extern SyncContext g_sync;

/*
 * Inicializa todos os recursos utilizados pela sincronização.
 *
 * Esta função cria os mutexes responsáveis pelas células, identifica os cruzamentos presentes 
 * no mapa e inicializa os semáforos.
 */
void sync_init(Map *map);

/*
 * Liberação dos recursos.
 */
void sync_destroy(void);

/*
 * Retorna o semáforo associado ao cruzamento
 * localizado em (row, col).
 *
 * Retorna NULL caso a posição não corresponda
 * a um cruzamento.
 */
const TrafficSignal *sync_get_signal(
    int row,
    int col);

/*
 * Tenta ocupar uma célula.
 *
 * Retorna:
 *      1 -> sucesso
 *      0 -> célula ocupada
 */
int cell_try_occupy(
    Map *map,
    int row,
    int col,
    int vehicle_id);

/*
 * Libera uma célula que antes era ocupada por um veículo, para indicar que o espaço pode ser ocupado.
 */
void cell_release(
    Map *map,
    int row,
    int col);

/*
 * Inicializa um semáforo de cruzamento.
 */
void traffic_signal_init(
    int index,
    int row,
    int col);

/*
 * Bloqueia a thread do veículo enquanto o semáforo permanecer fechado para a direção informada.
 *
 * A execução continua automaticamente quando o semáforo correspondente liberar a passagem.
 */
void traffic_wait_green(
    int row,
    int col,
    Direction direction);

/*
 * Thread responsável pelo ciclo do semáforo.
 */
void *traffic_signal_loop(
    void *arg);

/*
 * Solicita prioridade para um veículo que deseja atravessar um cruzamento.
 *
 * A implementação utilizará essa informação para alterar dinamicamente o ciclo do semáforo.
 */
void traffic_request_priority(
    int row,
    int col,
    Direction direction);

#endif