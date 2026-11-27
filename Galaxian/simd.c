#include "simd.h"
#include "bullet.h"
#include "game.h"
#include <emmintrin.h>   /* SSE2 intrinsics: __m128i, _mm_add_epi32 и т.д. */

/* =====================================================================
 * МОДУЛЬ ВЕКТОРИЗОВАННЫХ ВЫЧИСЛЕНИЙ (SSE2)
 *
 * В MSVC для x64 встроенный ассемблер (__asm) не поддерживается, поэтому
 * SIMD реализован через intrinsics из <emmintrin.h>. Компилятор превращает
 * _mm_add_epi32 в p4addd, _mm_cmpgt_epi32 в pcmpgtd ровно так же, как это
 * было бы написано на ассемблере.
 *
 * Глобальный флаг g_use_simd позволяет в бенчмарке переключаться между
 * векторизованной и исходной скалярной версиями в одном прогоне.
 * ===================================================================== */

int g_use_simd = 1;

/* Индексы активных пуль, разделённые по принадлежности.
 * Нужны, чтобы у всех 4 пуль в блоке SIMD-цикла было одно направление полёта,
 * и внутри векторизованного кода не было ветвления по is_enemy. */
static int idx_own[MAX_BULLETS];
static int idx_enemy[MAX_BULLETS];

/* ---------------------------------------------------------------------
 * Исходная скалярная версия — точно та же логика, что была в bullet.c
 * до оптимизации. Нужна как база для сравнения в одном прогоне.
 * --------------------------------------------------------------------- */
static void update_bullets_scalar(void) {
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

/* ---------------------------------------------------------------------
 * ВЕКТОРИЗОВАННАЯ ВЕРСИЯ
 *
 * Шаг 1: разложить активные пули на две группы (свои / вражеские).
 * Шаг 2: обработать каждую группу блоками по 4 пули:
 *        - загрузить четыре y в один xmm-регистр (128 бит = 4 x int32);
 *        - сложить/вычесть BULLET_SPEED одной инструкцией (paddd);
 *        - проверить границу экрана одной инструкцией (pcmpgtd);
 *        - записать y обратно и выключить вышедшие за экран пули.
 * Хвост (менее 4 пуль) обрабатывается скалярно.
 * --------------------------------------------------------------------- */
static void update_bullets_simd(void) {
    int n_own = 0, n_enemy = 0;

    /* Шаг 1: один проход — разложить активные пули по двум спискам */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        if (bullets[i].is_enemy) idx_enemy[n_enemy++] = i;
        else                     idx_own[n_own++] = i;
    }

    /* Шаг 2a: свои пули летят вверх (y -= BULLET_SPEED).
     * _mm_set1_epi32 размножает константу в 4 слота регистра. */
    {
        const __m128i step_up = _mm_set1_epi32(-BULLET_SPEED);
        const __m128i zero = _mm_setzero_si128();

        for (int k = 0; k + 4 <= n_own; k += 4) {
            int i0 = idx_own[k], i1 = idx_own[k + 1],
                i2 = idx_own[k + 2], i3 = idx_own[k + 3];

            /* Четыре y в один регистр. Поля лежат с шагом sizeof(Bullet),
             * поэтому одной загрузкой из памяти их не взять — компилятор
             * соберёт регистр из четырёх mov. Экономия идёт на арифметике
             * и на отсутствии ветвлений, а не на чтении. */
            __m128i ys = _mm_setr_epi32(bullets[i0].y, bullets[i1].y,
                bullets[i2].y, bullets[i3].y);
            ys = _mm_add_epi32(ys, step_up);        /* y -= BULLET_SPEED, 4 сразу */

            /* y < 0  <=>  0 > y  (знаковое сравнение, pcmgtd) */
            __m128i out = _mm_cmpgt_epi32(zero, ys);

            bullets[i0].y = _mm_cvtsi128_si32(ys); ys = _mm_srli_si128(ys, 4);
            bullets[i1].y = _mm_cvtsi128_si32(ys); ys = _mm_srli_si128(ys, 4);
            bullets[i2].y = _mm_cvtsi128_si32(ys); ys = _mm_srli_si128(ys, 4);
            bullets[i3].y = _mm_cvtsi128_si32(ys);

            /* Маска -> выключение. 0xFFFF — ненулевой «ложный» int, лишь бы
             * было понятно, что бит выставлен; проверяем просто != 0. */
            int m0 = (_mm_cvtsi128_si32(out)) & 0xFFFF;
            int m1 = (_mm_cvtsi128_si32(_mm_srli_si128(out, 4))) & 0xFFFF;
            int m2 = (_mm_cvtsi128_si32(_mm_srli_si128(out, 8))) & 0xFFFF;
            int m3 = (_mm_cvtsi128_si32(_mm_srli_si128(out, 12))) & 0xFFFF;
            if (m0) bullets[i0].active = false;
            if (m1) bullets[i1].active = false;
            if (m2) bullets[i2].active = false;
            if (m3) bullets[i3].active = false;
        }

        /* Хвост: меньше 4 пуль */
        for (int k = (n_own / 4) * 4; k < n_own; k++) {
            int i = idx_own[k];
            bullets[i].y -= BULLET_SPEED;
            if (bullets[i].y < 0) bullets[i].active = false;
        }
    }

    /* Шаг 2b: вражеские пули летят вниз (y += BULLET_SPEED) */
    {
        const __m128i step_down = _mm_set1_epi32(BULLET_SPEED);
        const __m128i limit = _mm_set1_epi32(SCREEN_HEIGHT);

        for (int k = 0; k + 4 <= n_enemy; k += 4) {
            int i0 = idx_enemy[k], i1 = idx_enemy[k + 1],
                i2 = idx_enemy[k + 2], i3 = idx_enemy[k + 3];

            __m128i ys = _mm_setr_epi32(bullets[i0].y, bullets[i1].y,
                bullets[i2].y, bullets[i3].y);
            ys = _mm_add_epi32(ys, step_down);      /* y += BULLET_SPEED, 4 сразу */

            /* y > SCREEN_HEIGHT (знаковое сравнение) */
            __m128i out = _mm_cmpgt_epi32(ys, limit);

            bullets[i0].y = _mm_cvtsi128_si32(ys); ys = _mm_srli_si128(ys, 4);
            bullets[i1].y = _mm_cvtsi128_si32(ys); ys = _mm_srli_si128(ys, 4);
            bullets[i2].y = _mm_cvtsi128_si32(ys); ys = _mm_srli_si128(ys, 4);
            bullets[i3].y = _mm_cvtsi128_si32(ys);

            int m0 = (_mm_cvtsi128_si32(out)) & 0xFFFF;
            int m1 = (_mm_cvtsi128_si32(_mm_srli_si128(out, 4))) & 0xFFFF;
            int m2 = (_mm_cvtsi128_si32(_mm_srli_si128(out, 8))) & 0xFFFF;
            int m3 = (_mm_cvtsi128_si32(_mm_srli_si128(out, 12))) & 0xFFFF;
            if (m0) bullets[i0].active = false;
            if (m1) bullets[i1].active = false;
            if (m2) bullets[i2].active = false;
            if (m3) bullets[i3].active = false;
        }

        /* Хвост */
        for (int k = (n_enemy / 4) * 4; k < n_enemy; k++) {
            int i = idx_enemy[k];
            bullets[i].y += BULLET_SPEED;
            if (bullets[i].y > SCREEN_HEIGHT) bullets[i].active = false;
        }
    }
}

/* Точка входа для bullet.c: выбирает реализацию по флагу. */
void simd_update_bullets(void) {
    if (g_use_simd) update_bullets_simd();
    else            update_bullets_scalar();
}