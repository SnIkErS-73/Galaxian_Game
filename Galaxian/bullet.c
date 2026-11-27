#include "bullet.h"
#include "render.h"
#include "game.h"
#include "simd.h"

__declspec(align(64)) Bullet bullets[MAX_BULLETS];

void init_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
}

/* Логика движения вынесена в simd.c; версия выбирается флагом g_use_simd. */
void update_bullets(void) {
    simd_update_bullets();
}

void draw_bullet(Bullet* b) {
    if (!b->active) return;

    batch_add(b->is_enemy ? PAL_RED : PAL_YELLOW,
        b->x, b->y, b->width, b->height);
}