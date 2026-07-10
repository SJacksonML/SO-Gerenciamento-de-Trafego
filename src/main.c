/*
 * main.c — Ponto de entrada do simulador.
 *
 * Responsabilidade (Icaro): inicializar todos os módulos, lançar as threads
 * (relógio, semáforos e veículos) e conduzir o loop de renderização até o
 * fim da simulação, encerrando tudo de forma limpa (join + destroy).
 *
 * ATENÇÃO — dependência conhecida:
 *   Este arquivo só compila depois que include/vehicle.h passar a incluir
 *   "map.h" (Direction e Map são usados em vehicle.h mas não estão
 *   declarados ali). Essa correção faz parte da tarefa separada de arrumar
 *   src/vehicle.c e não foi feita aqui de propósito.
 *
 * Uso:
 *   ./bin/simulador [caminho_do_mapa] [ticks_da_simulacao]
 *
 * Se nenhum argumento for passado, usa assets/map.txt e roda por
 * SIMULATION_TICKS ticks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "map.h"
#include "sync.h"
#include "clock.h"
#include "vehicle.h"
#include "render.h"

/* ------------------------------------------------------------------ */
/* Configuração da simulação                                          */
/* ------------------------------------------------------------------ */

#define DEFAULT_MAP_PATH "assets/map.txt"

/*
 * vehicle_run() hoje tem uma condição de parada fixa em 100 ticks
 * (ver src/vehicle.c). Damos uma margem antes de mandar o relógio parar,
 * pra dar tempo dos carros perceberem o fim e saírem do loop sozinhos
 * antes que o broadcast do relógio pare de acontecer.
 */
#define VEHICLE_LIFETIME_TICKS 100
#define SIMULATION_TICKS (VEHICLE_LIFETIME_TICKS + 50)

/* Quantos veículos lançar. Respeita o limite atual de vehicle.h.
 * OBS: a especificação pede de 10 a 20 carros — MAX_VEHICLE precisa
 * subir para pelo menos 20 quando o módulo de veículos for revisado. */
#define NUM_TOTAL_VEHICLES MAX_VEHICLE
#define NUM_AMBULANCES 1
#define NUM_CARS (NUM_TOTAL_VEHICLES - NUM_AMBULANCES)

/* ------------------------------------------------------------------ */
/* Utilitário local: pontos de partida válidos                        */
/* ------------------------------------------------------------------ */

typedef struct {
    int row;
    int col;
    Direction dir;
} SpawnPoint;

/*
 * Varre o mapa carregado e coleta células de via de mão única, que
 * servem como pontos de partida naturais (já indicam a direção inicial
 * do veículo). Cruzamentos e paredes não são usados como spawn.
 */
static int collect_spawn_points(const Map *m, SpawnPoint *out, int max_count) {
    int found = 0;

    for (int r = 0; r < m->rows && found < max_count; r++) {
        for (int c = 0; c < m->cols && found < max_count; c++) {
            Direction dir;
            switch (m->grid[r][c].type) {
                case CELL_ONE_WAY_N: dir = DIR_NORTH; break;
                case CELL_ONE_WAY_S: dir = DIR_SOUTH; break;
                case CELL_ONE_WAY_E: dir = DIR_EAST;  break;
                case CELL_ONE_WAY_W: dir = DIR_WEST;  break;
                default: continue; /* não é ponto de partida válido */
            }

            out[found].row = r;
            out[found].col = c;
            out[found].dir = dir;
            found++;
        }
    }

    return found;
}

/*
 * pthread_create exige uma função void *(*)(void *), mas vehicle_run()
 * está declarada como void vehicle_run(Vehicle *). Este wrapper faz a
 * ponte sem precisar tocar em vehicle.c agora.
 */
static void *vehicle_thread_entry(void *arg) {
    Vehicle *v = (Vehicle *)arg;
    vehicle_run(v);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    const char *map_path = (argc > 1) ? argv[1] : DEFAULT_MAP_PATH;
    int sim_ticks = (argc > 2) ? atoi(argv[2]) : SIMULATION_TICKS;

    /* ---------- 1. Mapa ---------- */
    Map map;
    if (map_load(&map, map_path) != 0) {
        fprintf(stderr, "[main] falha ao carregar o mapa '%s'\n", map_path);
        return EXIT_FAILURE;
    }

    /* ---------- 2. Sincronização (mutex de células + semáforos) ---------- */
    sync_init(&map);

    /* ---------- 3. Relógio global ---------- */
    clock_init();

    pthread_t clock_tid;
    if (pthread_create(&clock_tid, NULL, clock_thread, NULL) != 0) {
        fprintf(stderr, "[main] falha ao criar a thread do relogio\n");
        return EXIT_FAILURE;
    }

    /* ---------- 4. Threads de semáforo (uma por cruzamento) ---------- */
    pthread_t signal_tids[MAX_INTERSECTIONS];
    int num_signals = g_sync.num_signals;

    for (int i = 0; i < num_signals; i++) {
        if (pthread_create(&signal_tids[i], NULL, traffic_signal_loop,
                            &g_sync.signals[i]) != 0) {
            fprintf(stderr, "[main] falha ao criar thread do semaforo %d\n", i);
        }
    }

    /* ---------- 5. Veículos (carros + ambulância) ---------- */
    SpawnPoint spawns[MAX_ROWS * MAX_COLS];
    int num_spawns = collect_spawn_points(&map, spawns, MAX_ROWS * MAX_COLS);

    if (num_spawns == 0) {
        fprintf(stderr, "[main] mapa sem vias de mao unica para servir de spawn\n");
        return EXIT_FAILURE;
    }

    Vehicle vehicles[NUM_TOTAL_VEHICLES];
    pthread_t vehicle_tids[NUM_TOTAL_VEHICLES];

    Speed speed_cycle[3] = { SPEED_SLOW, SPEED_MEDIUM, SPEED_FAST };
    int spawn_cursor = 0;
    int launched = 0;

    for (int i = 0; i < NUM_TOTAL_VEHICLES; i++) {
        VehicleType type = (i < NUM_AMBULANCES) ? TYPE_AMBULANCE : TYPE_CAR;
        Speed speed = (type == TYPE_AMBULANCE) ? SPEED_FAST
                                                : speed_cycle[i % 3];

        /* Procura um ponto de partida livre, avançando pelo vetor de
         * spawns até achar uma célula desocupada (evita dois veículos
         * nascendo na mesma célula). */
        int reserved = 0;
        for (int attempt = 0; attempt < num_spawns; attempt++) {
            SpawnPoint sp = spawns[spawn_cursor % num_spawns];
            spawn_cursor++;

            if (cell_try_occupy(&map, sp.row, sp.col, i)) {
                Pos start = { sp.row, sp.col };
                vehicle_init(&vehicles[i], i, type, sp.dir, start, speed, &map);
                reserved = 1;
                break;
            }
        }

        if (!reserved) {
            fprintf(stderr, "[main] nao foi possivel achar spawn livre para veiculo %d\n", i);
            continue;
        }

        if (pthread_create(&vehicle_tids[i], NULL, vehicle_thread_entry,
                            &vehicles[i]) != 0) {
            fprintf(stderr, "[main] falha ao criar thread do veiculo %d\n", i);
            cell_release(&map, vehicles[i].pos.row, vehicles[i].pos.col);
            continue;
        }

        launched++;
    }

    printf("[main] simulacao iniciada com %d veiculo(s) (%d ambulancia(s)), %d semaforo(s)\n",
           launched, NUM_AMBULANCES, num_signals);

    /* ---------- 6. Loop principal: renderiza a cada tick ---------- */
    int tick;
    while ((tick = clock_get_tick()) < sim_ticks) {
        render_frame(&map);
        clock_wait_tick();
    }
    render_frame(&map); /* último frame antes de encerrar */

    /* ---------- 7. Encerramento ---------- */
    clock_stop();
    sync_wakeup_all();

    pthread_join(clock_tid, NULL);

    for (int i = 0; i < NUM_TOTAL_VEHICLES; i++) {
        if (i < launched) {
            pthread_join(vehicle_tids[i], NULL);
        }
    }

    for (int i = 0; i < num_signals; i++) {
        pthread_join(signal_tids[i], NULL);
    }

    sync_destroy();
    clock_destroy();
    map_free(&map);

    printf("[main] simulacao encerrada apos %d ticks\n", tick);
    return EXIT_SUCCESS;
}
