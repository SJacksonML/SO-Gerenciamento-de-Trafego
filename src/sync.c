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
            pthread_mutex_destroy(
                &g_sync.cell_mutex[row][col]);
        }
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
    if (index < 0 || index >= N_INTERSECTIONS)
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
    pthread_mutex_lock(
        &g_sync.cell_mutex[row][col]);
    if (map->grid[row][col].occupant_id != -1)
    {
        pthread_mutex_unlock(
            &g_sync.cell_mutex[row][col]);
        return 0;
    }

    map->grid[row][col].occupant_id = vehicle_id;

    pthread_mutex_unlock(
        &g_sync.cell_mutex[row][col]);
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
    pthread_mutex_lock(
        &g_sync.cell_mutex[row][col]);

    map->grid[row][col].occupant_id = -1;

    pthread_mutex_unlock(
        &g_sync.cell_mutex[row][col]);
}

/* ------------- = ------------- = ------------- = ------------- = ------------- = ------------- */

/*
 * As funções abaixo pertencem à próxima etapa do projeto.
 * Permanecem como stubs para permitir a compilação do sistema.
 */

 /* STUB
 * Inicializa todas as estruturas associadas a um semáforo de cruzamento.
 *
 * Cada semáforo possui mecanismos próprios de sincronização, permitindo que 
 * veículos aguardem pela abertura do sinal sem realizar espera ativa.
 */
void traffic_signal_init(
    int signal_index,
    int row,
    int col)
{
    TrafficSignal *signal = &g_sync.signals[signal_index];

    signal->row = row;
    signal->col = col;

    pthread_mutex_init(&signal->mutex, NULL);
    pthread_cond_init(&signal->cond, NULL);

    for (int dir = 0; dir < DIR_COUNT; dir++)
    {
        signal->state[dir] = SIGNAL_RED;
    }

    signal->state[DIR_UP] = SIGNAL_GREEN;
    signal->state[DIR_DOWN] = SIGNAL_GREEN;
}

void traffic_wait_green(
    int row,
    int col,
    Direction direction)
{
    (void)row;
    (void)col;
    (void)direction;
}

void *traffic_signal_loop(
    void *arg)
{
    (void)arg;

    return NULL;
}

void traffic_request_priority(
    int row,
    int col,
    Direction direction)
{
    (void)row;
    (void)col;
    (void)direction;
}