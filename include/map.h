//
// Created by Aisha on 03/07/2026.
//

#ifndef MAP_H
#define MAP_H

#define MAX_ROWS 30
#define MAX_COLS 30
#define MAX_INTERSECTIONS 64


typedef enum
{
    DIR_INVALID = -1,
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


typedef struct {
    CellType type;
    int occupant_id;
    char occupant_symbol;
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
int map_load(Map *map, const char *path);

void map_free(Map *m);

int map_is_valid_move(const Map *m, int row, int col, Direction dir);


/* Varre o mapa já carregado e preenche com a posição do CELL_INTERSECTION */
int map_get_intersections(const Map *m, int out_rows[], int out_cols[], int max_count);

#endif
