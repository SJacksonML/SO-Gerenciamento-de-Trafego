#include <stdio.h>
#include <unistd.h>
#include "sync.h"

/*
 * Instância única do contexto global de sincronização.
 */
SyncContext g_sync;

/*
 * Localiza todos os cruzamentos existentes no mapa, registra sua posição
 * e cria as estruturas de sincronização correspondentes.
 */
void sync_init(Map *map)
{
    /* Inicializa os mutexes das células */
    for (int row = 0; row < MAX_ROWS; row++)
    {
        for (int col = 0; col < MAX_COLS; col++)
        {
            pthread_mutex_init(&g_sync.cell_mutex[row][col], NULL);

            /* Inicialmente, nenhuma célula possui um semáforo associado */
            g_sync.signal_index[row][col] = -1;
        }
    }

    /* Inicializa os semáforos */
    int signal_index = 0;

    for (int row = 0; row < MAX_ROWS; row++)
    {
        for (int col = 0; col < MAX_COLS; col++)
        {
            if (map->grid[row][col].type == CELL_INTERSECTION)
            {
                /* Associa esta posição ao índice do vetor de semáforos */
                g_sync.signal_index[row][col] = signal_index;

                /* Inicializa o semáforo correspondente */
                traffic_signal_init(signal_index, row, col);
                signal_index++;
            }
        }
    }
    g_sync.num_signals = signal_index;
}

/*
 * Libera todos os mutexes criados em sync_init().
 */
void sync_destroy(void)
{
    for (int row = 0; row < MAX_ROWS; row++)
    {
        for (int col = 0; col < MAX_COLS; col++)
        {
            pthread_mutex_destroy(&g_sync.cell_mutex[row][col]);
        }
    }

    for (int i = 0; i < g_sync.num_signals; i++)
    {
        pthread_mutex_destroy(&g_sync.signals[i].mutex);
        pthread_cond_destroy(&g_sync.signals[i].cond);
    }
}

/* 
 * Obtém o semáforo associado a uma posição do mapa.
 */
const TrafficSignal *sync_get_signal(
    int row,
    int col)
{
    if (row < 0 || row >= MAX_ROWS ||
        col < 0 || col >= MAX_COLS)
    {
        return NULL;
    }
    int index = g_sync.signal_index[row][col];
    if (index < 0 || index >= g_sync.num_signals)
    {
        return NULL; /* A posição não corresponde a um cruzamento. */
    }
    return &g_sync.signals[index];
}

/*
 * Tenta ocupar uma célula do mapa.
 *
 * Retorna:
 *      1 -> ocupação realizada
 *      0 -> célula já ocupada
 */
int cell_try_occupy(
    Map *map,
    int row,
    int col,
    int vehicle_id)
{
    pthread_mutex_lock(&g_sync.cell_mutex[row][col]);
    if (map->grid[row][col].occupant_id != -1)
    {
        pthread_mutex_unlock(&g_sync.cell_mutex[row][col]);
        return 0;
    }

    map->grid[row][col].occupant_id = vehicle_id;

    pthread_mutex_unlock(&g_sync.cell_mutex[row][col]);
    return 1;
}

/*
 * Libera uma célula previamente ocupada.
 */
void cell_release(
    Map *map,
    int row,
    int col)
{
    pthread_mutex_lock(&g_sync.cell_mutex[row][col]);

    int was_ambulance = (map->grid[row][col].occupant_symbol == 'A');
    map->grid[row][col].occupant_id = -1;
    map->grid[row][col].occupant_symbol = '\0';

    pthread_mutex_unlock(&g_sync.cell_mutex[row][col]);

    /* Se for cruzamento e a ambulância estiver liberando, limpa a prioridade */
    if (map->grid[row][col].type == CELL_INTERSECTION && was_ambulance)
    {
        int idx = g_sync.signal_index[row][col];
        if (idx >= 0 && idx < g_sync.num_signals)
        {
            TrafficSignal *signal = &g_sync.signals[idx];
            pthread_mutex_lock(&signal->mutex);
            signal->priority_requested = 0;
            pthread_mutex_unlock(&signal->mutex);
        }
    }
}

/* ------------- = ------------- = ------------- = ------------- = ------------- = ------------- */

/*
 * Inicializa todas as estruturas associadas a um semáforo de cruzamento.
 */
void traffic_signal_init(
    int index,
    int row,
    int col)
{
    TrafficSignal *signal = &g_sync.signals[index];

    signal->row = row;
    signal->col = col;
    signal->priority_requested = 0;
    signal->priority_dir = DIR_INVALID;

    pthread_mutex_init(&signal->mutex, NULL);
    pthread_cond_init(&signal->cond, NULL);

    for (int dir = 0; dir < DIR_COUNT; dir++)
    {
        signal->state[dir] = SIGNAL_RED;
    }

    /* Começa verde no sentido norte-sul */
    signal->state[DIR_NORTH] = SIGNAL_GREEN;
    signal->state[DIR_SOUTH] = SIGNAL_GREEN;
}

void traffic_wait_green(
    int row,
    int col,
    Direction direction)
{
    const TrafficSignal *signal = sync_get_signal(row, col);
    if (!signal) return;

    TrafficSignal *sig = (TrafficSignal *)signal;

    pthread_mutex_lock(&sig->mutex);
    while (sig->state[direction] == SIGNAL_RED)
    {
        pthread_cond_wait(&sig->cond, &sig->mutex);
    }
    pthread_mutex_unlock(&sig->mutex);
}

void *traffic_signal_loop(
    void *arg)
{
    TrafficSignal *signal = (TrafficSignal *)arg;
    extern int is_active;

    while (is_active)
    {
        pthread_mutex_lock(&signal->mutex);
        if (signal->priority_requested)
        {
            for (int d = 0; d < DIR_COUNT; d++)
            {
                signal->state[d] = (d == signal->priority_dir) ? SIGNAL_GREEN : SIGNAL_RED;
            }
            pthread_cond_broadcast(&signal->cond);
            pthread_mutex_unlock(&signal->mutex);
            usleep(100000);
            continue;
        }

        /* Fase 1: Norte e Sul verde, Leste e Oeste vermelho */
        signal->state[DIR_NORTH] = SIGNAL_GREEN;
        signal->state[DIR_SOUTH] = SIGNAL_GREEN;
        signal->state[DIR_EAST] = SIGNAL_RED;
        signal->state[DIR_WEST] = SIGNAL_RED;
        pthread_cond_broadcast(&signal->cond);
        pthread_mutex_unlock(&signal->mutex);

        for (int i = 0; i < 20 && is_active; i++)
        {
            pthread_mutex_lock(&signal->mutex);
            if (signal->priority_requested)
            {
                pthread_mutex_unlock(&signal->mutex);
                break;
            }
            pthread_mutex_unlock(&signal->mutex);
            usleep(100000);
        }

        pthread_mutex_lock(&signal->mutex);
        if (signal->priority_requested)
        {
            pthread_mutex_unlock(&signal->mutex);
            continue;
        }

        /* Fase 2: Norte e Sul vermelho, Leste e Oeste verde */
        signal->state[DIR_NORTH] = SIGNAL_RED;
        signal->state[DIR_SOUTH] = SIGNAL_RED;
        signal->state[DIR_EAST] = SIGNAL_GREEN;
        signal->state[DIR_WEST] = SIGNAL_GREEN;
        pthread_cond_broadcast(&signal->cond);
        pthread_mutex_unlock(&signal->mutex);

        for (int i = 0; i < 20 && is_active; i++)
        {
            pthread_mutex_lock(&signal->mutex);
            if (signal->priority_requested)
            {
                pthread_mutex_unlock(&signal->mutex);
                break;
            }
            pthread_mutex_unlock(&signal->mutex);
            usleep(100000);
        }
    }

    return NULL;
}

void traffic_request_priority(
    int row,
    int col,
    Direction direction)
{
    const TrafficSignal *signal = sync_get_signal(row, col);
    if (!signal) return;

    TrafficSignal *sig = (TrafficSignal *)signal;
    pthread_mutex_lock(&sig->mutex);
    if (!sig->priority_requested || sig->priority_dir != direction)
    {
        sig->priority_requested = 1;
        sig->priority_dir = direction;

        for (int d = 0; d < DIR_COUNT; d++)
        {
            sig->state[d] = (d == direction) ? SIGNAL_GREEN : SIGNAL_RED;
        }

        printf("\n>>> [PRIORIDADE] Ambulancia solicitou prioridade no cruzamento (%d, %d) indo para %s! <<<\n",
               row, col,
               direction == DIR_NORTH ? "NORTE" :
               direction == DIR_SOUTH ? "SUL" :
               direction == DIR_EAST ? "LESTE" :
               direction == DIR_WEST ? "OESTE" : "INVALIDO");
        fflush(stdout);

        pthread_cond_broadcast(&sig->cond);
    }
    pthread_mutex_unlock(&sig->mutex);
}