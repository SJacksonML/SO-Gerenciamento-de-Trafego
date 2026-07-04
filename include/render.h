//
// Created by Aisha on 03/07/2026.
//

#ifndef RENDER_H
#define RENDER_H

#include "map.h"
#include "sync.h"

/*
 * Limpa o terminal e redesenha o mapa inteiro,
 * incluindo veículos e os semáforos
 *
 * TODO: (Icaro) Isso é chamado a cada tick do relógio.
 *
 */

void render_frame(const Map *m);

#endif
