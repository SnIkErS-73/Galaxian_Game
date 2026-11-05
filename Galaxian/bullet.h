#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include "game.h"

typedef struct {
    int x, y;
    int width, height;
    bool active;
    bool is_enemy;
} Bullet;

extern Bullet bullets[MAX_BULLETS];

void init_bullets(void);
void update_bullets(void);
void draw_bullet(Bullet* b);

#endif