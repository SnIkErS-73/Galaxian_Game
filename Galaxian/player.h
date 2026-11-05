#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "game.h"  // Добавить для SCREEN_WIDTH, PLAYER_Y

typedef struct {
    int x, y;
    int width, height;
    bool alive;
} Player;

extern Player player;

void init_player(void);
void draw_player(void);

#endif