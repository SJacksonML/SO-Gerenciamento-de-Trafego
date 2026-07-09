#include <stdio.h>
#include "../include/map.h"
#include "../include/render.h"

/* Interna: decide o caractere ASCII de uma célula.
 * Prioridade de desenho: veículo ocupando > estado do semáforo > tipo da via. */

static char cell_char(const Map *m, int r, int c) {
    const Cell *cell = &m->grid[r][c];

    if (cell->occupant_id != -1) {

        /* Há um veículo aqui. occupant_symbol é escrito por quem ocupa
         * a célula (try_move / cell_try_occupy, no módulo do Elilúcio),
         * por exemplo 'A' para ambulância ou 'C'/dígito para carro comum.
         * Se occupant_id != -1 mas ninguém setou occupant_symbol ainda,
         * caímos em '?' — isso é sinal de bug em outro módulo (célula
         * marcada como ocupada sem símbolo de exibição definido). */


        return cell->occupant_symbol != '\0' ? cell->occupant_symbol : '?';
    }

    switch (cell->type) {
        case CELL_WALL:
            return '#';
        case CELL_INTERSECTION:
            /* O estado do semáforo (R/G) é gerenciado pelo módulo de
             * sincronização (Jackson), não pelo Map. Aqui desenhamos
             * só o "traço" do cruzamento; se a equipe quiser ver R/G
             * na tela, o Jackson precisa expor uma forma de consultar
             * o estado (ex: uma função tipo traffic_signal_get_state()
             * em sync.h) e passar isso pro render de algum jeito —> não
             * é algo que dá pra resolver só com o que Map guarda. */
            return '+';
        case CELL_ONE_WAY_N:
            return '^';
        case CELL_ONE_WAY_S:
            return 'v';
        case CELL_ONE_WAY_E:
            return '>';
        case CELL_ONE_WAY_W:
            return '<';
        default:
            return '?';
    }
}

void render_frame(const Map *m) {
    if (!m) return;

    /* \033[2J limpa a tela inteira; \033[H move o cursor pro topo.
     * Fazer os dois nessa ordem é o padrão pra "clear + home" — a
     * tentativa anterior de usar só \033[H\033[J na prática já limpava
     * a tela inteira (porque \033[J limpa "do cursor até o fim", e o
     * cursor já estava no topo), então deixamos isso explícito. */

    printf("\033[2J\033[H");

    for (int r = 0; r < m->rows; r++) {
        for (int c = 0; c < m->cols; c++) {
            putchar(cell_char(m, r, c));
        }
        putchar('\n');
    }

    /* A legenda só precisa aparecer uma vez, não a cada tick — repeti-la
     * em todo frame só gera saída redundante e "pisca" texto embaixo do
     * mapa sem necessidade. */
    static int legend_printed = 0;
    if (!legend_printed) {
        printf("Legenda: # parede | + cruzamento | ^v<> faixa (mao unica; par ^v ou <> lado a lado = mao dupla em 2 faixas) | A ambulancia\n");
        legend_printed = 1;
    }

    fflush(stdout);
}
