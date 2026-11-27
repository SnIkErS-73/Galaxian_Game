#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include "game.h"

typedef struct {
    int x, y;               /* горячие: движение и проверка столкновений */
    bool active;            /* горячие */
    bool is_enemy;          /* горячие: ветвление в check_collisions */
    int width, height;      /* горячие */
} Bullet;

extern Bullet bullets[MAX_BULLETS];

void init_bullets(void);
void update_bullets(void);
void draw_bullet(Bullet* b);

#endif