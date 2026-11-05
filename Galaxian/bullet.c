#include "bullet.h"
#include "render.h"
#include "game.h"

Bullet bullets[MAX_BULLETS];

void init_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
}

void update_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            if (bullets[i].is_enemy) {
                bullets[i].y += BULLET_SPEED;
                if (bullets[i].y > SCREEN_HEIGHT) {
                    bullets[i].active = false;
                }
            }
            else {
                bullets[i].y -= BULLET_SPEED;
                if (bullets[i].y < 0) {
                    bullets[i].active = false;
                }
            }
        }
    }
}

void draw_bullet(Bullet* b) {
    if (!b->active) return;

    Color c = b->is_enemy ? COLOR_RED : COLOR_YELLOW;
    draw_rect(b->x, b->y, b->width, b->height, c);
}