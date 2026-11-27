#ifndef SIMD_H
#define SIMD_H

/* Модуль векторизованных вычислений, g_use_simd = 1 — векторизованная версия, 0 — исходная скалярная. */
extern int g_use_simd;

void simd_update_bullets(void);

#endif