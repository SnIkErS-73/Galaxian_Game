#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "render.h"
#include "game.h"

typedef struct {
    int x, y;
    int width, height;
    bool alive;
    bool diving;
    int dive_target_x;
    int dive_step;
    Color color;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];

void init_enemies(void);
void update_enemies(void);
void draw_enemy(Enemy* e);
bool check_victory(void);

#endif