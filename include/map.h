//
// Created by Aisha on 03/07/2026.
//

#ifndef MAP_H
#define MAP_H

#define MAX_ROWS 30
#define MAX_COLS 30

typedef enum
{
    DIR_NORTH = 0,
    DIR_SOUTH,
    DIR_EAST,
    DIR_WEST,
    DIR_COUNT
} Direction;

typedef enum
{
    CELL_WALL = 0,
    CELL_ROAD,
    CELL_INTERSECTION,
    CELL_ONE_WAY_N,
    CELL_ONE_WAY_S,
    CELL_ONE_WAY_E,
    CELL_ONE_WAY_W
} CellType;


/* TODO: (Jackson) Quem muda esses valores é o módulo de sincronização*/
typedef enum {
    SIGNAL_NONE  = -1, /* Não existe semáforo aqui */
    SIGNAL_GREEN = 0,
    SIGNAL_RED   = 1
} SignalState;

typedef struct {
    CellType type;
    int occupant_id;
    char occupant_symbol;

    SignalState signal_state;
} Cell;

typedef struct
{
    Cell grid [MAX_ROWS][MAX_COLS];
    int rows;
    int cols;
} Map;

/* (AISHA)
 *
 * Lê o arquivo .txt
 * Retorna 0 em sucesso, -1 em erro.
 *
 */
int map_load(Map *map, char *path);

void map_free(Map *map);

int map_is_valid_move(const Map *m, int row, int col, Direction dir);


#endif
