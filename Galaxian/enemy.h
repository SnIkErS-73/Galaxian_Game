#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "render.h"
#include "game.h"

/* Структура Enemy с переставленными полями.
 *
 * МАШИННО-ЗАВИСИМАЯ ОПТИМИЗАЦИЯ: локализация горячих данных.
 *
 * Процессор читает память строками кэша (64 байта). Поля, которые
 * читаются в одном цикле, надо держать рядом, чтобы они попали
 * в одну кэш-строку. Редко используемые поля отодвигаем в конец.
 *
 * Порядок:
 *   1) x, y, alive, width, height - читаются КАЖДЫЙ КАДР
 *      (движение, проверка столкновений).
 *   2) diving, dive_step, dive_target_x - только во время пикирования.
 *   3) color, palette_idx - только при отрисовке.
 */
typedef struct {
    int x, y;               /* горячие */
    bool alive;             /* горячие */
    int width, height;      /* горячие */
    bool diving;            /* холодные */
    int dive_step;          /* холодные */
    int dive_target_x;      /* холодные */
    Color color;            /* холодные (отрисовка) */
    int palette_idx;        /* холодные (отрисовка) */
} Enemy;

extern Enemy enemies[MAX_ENEMIES];

void init_enemies(void);
void update_enemies(void);
void draw_enemy(Enemy* e);
bool check_victory(void);

#endif