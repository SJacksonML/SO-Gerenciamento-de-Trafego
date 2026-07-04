//
// Created by Aisha on 03/07/2026.
//

#include <stdio.h>
#include <string.h>
#include "../include/map.h"

static CellType char_to_type(char c) {
    switch (c) {
    case '#': return CELL_WALL; /* PAREDE: Nenhum veículo entra aqui */
    case '.': return CELL_ROAD; /* ESTRADA: */
    case '+': return CELL_INTERSECTION; /* CRUZAMENTO: */
    case '^': return CELL_ONE_WAY_N; /* MÃO ÚNICA PARA O NORTE: */
    case 'v': return CELL_ONE_WAY_S; /* MÃO ÚNICA PARA O SUL: */
    case '>': return CELL_ONE_WAY_E; /* MÃO ÚNICA PARA O LESTE: */
    case '<': return CELL_ONE_WAY_W; /* MÃO ÚNICA PARA O OESTE :*/
    default:  return CELL_WALL;
    }
}

int map_load()
{

    }

void map_free(Map *map)
{

    }

int map_is_valid_move()
{

    }