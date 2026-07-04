#include <stdio.h>
#include <string.h>
#include "../include/map.h"

/* ------------------------------------------------------------------
 * Legenda do assets/map.txt:
 *   '#' parede / não é via
 *   '.' via comum (mão dupla)
 *   '+' cruzamento (semáforo)
 *   '^' via de mão única sentido norte (linha diminui)
 *   'v' via de mão única sentido sul (linha aumenta)
 *   '>' via de mão única sentido Leste (coluna aumenta)
 *   '<' via de mão única sentido oeste (coluna diminui)
 * ------------------------------------------------------------------ */

/* Interna: converte character do .txt para CellType. */
static CellType char_to_type(char c) {
    switch (c) {
        case '#': return CELL_WALL;
        case '.': return CELL_ROAD;
        case '+': return CELL_INTERSECTION;
        case '^': return CELL_ONE_WAY_N;
        case 'v': return CELL_ONE_WAY_S;
        case '>': return CELL_ONE_WAY_E;
        case '<': return CELL_ONE_WAY_W;
        default:  return CELL_WALL; /* character desconhecido = parede, por segurança */
    }
}

int map_load(Map *m, const char *path) {
    if (!m || !path) return -1;

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "[map_load] não foi possível abrir '%s'\n", path);
        return -1;
    }

    char line[MAX_COLS + 8]; /* + margem para '\n' e '\0' */
    int row = 0;
    int cols = -1;

    while (row < MAX_ROWS && fgets(line, sizeof(line), f)) {
        /* remove '\n' / '\r' do fim da linha */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        /* ignora linhas em branco no arquivo (ex: linha final vazia) */
        if (len == 0) continue;

        if (cols == -1) {
            cols = (int)len;
        } else if ((int)len != cols) {
            fprintf(stderr,
                "[map_load] linha %d tem %zu colunas, esperado %d (mapa deve ser retangular)\n",
                row, len, cols);
            fclose(f);
            return -1;
        }

        if (cols > MAX_COLS) {
            fprintf(stderr, "[map_load] mapa excede MAX_COLS (%d)\n", MAX_COLS);
            fclose(f);
            return -1;
        }

        for (int col = 0; col < cols; col++) {
            Cell *cell = &m->grid[row][col];
            cell->type = char_to_type(line[col]);
            cell->occupant_id = -1;
            cell->occupant_symbol = '\0'; /* '\0' = "sem símbolo definido ainda" */
        }

        row++;
    }

    fclose(f);

    if (row == 0 || cols <= 0) {
        fprintf(stderr, "[map_load] arquivo '%s' vazio ou inválido\n", path);
        return -1;
    }

    m->rows = row;
    m->cols = cols;
    return 0;
}

void map_free(Map *m) {
    if (!m) return;
    m->rows = 0;
    m->cols = 0;
    /* Grid é uma matriz estática (sem malloc), então não há free()
     * de memória aqui — apenas "zeramos" a struct logicamente. */
}

int map_is_valid_move(const Map *m, int row, int col, Direction dir) {
    if (!m) return 0;

    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) {
        return 0; /* fora do mapa */
    }

    const Cell *cell = &m->grid[row][col];

    if (cell->type == CELL_WALL) {
        return 0; /* não é via */
    }

    switch (cell->type) {
        case CELL_ONE_WAY_N:
            return dir == DIR_NORTH;
        case CELL_ONE_WAY_S:
            return dir == DIR_SOUTH;
        case CELL_ONE_WAY_E:
            return dir == DIR_EAST;
        case CELL_ONE_WAY_W:
            return dir == DIR_WEST;
        case CELL_ROAD:
        case CELL_INTERSECTION:
        default:
            return 1; /* mão dupla / cruzamento: qualquer direção é permitida aqui */
    }
}

int map_get_intersections(const Map *m, int out_rows[], int out_cols[], int max_count) {
    if (!m || !out_rows || !out_cols || max_count <= 0) return 0;

    int found = 0;
    for (int r = 0; r < m->rows && found < max_count; r++) {
        for (int c = 0; c < m->cols && found < max_count; c++) {
            if (m->grid[r][c].type == CELL_INTERSECTION) {
                out_rows[found] = r;
                out_cols[found] = c;
                found++;
            }
        }
    }
    return found;
}