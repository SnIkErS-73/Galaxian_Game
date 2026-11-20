#define _CRT_SECURE_NO_WARNINGS

#include "render.h"
#include "game.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

/* =====================================================================
 * БУФЕРЫ БАТЧИНГА
 *
 * Раньше каждый прямоугольник рисовался отдельным вызовом
 * SDL_RenderFillRect, и перед каждым менялся цвет через
 * SDL_SetRenderDrawColor. При 128 врагах это ~1000 вызовов SDL на кадр.
 *
 * Теперь прямоугольники складываются в буферы по цветам, и за кадр
 * выполняется 10 вызовов SDL_RenderFillRects - по одному на цвет.
 * ===================================================================== */
static SDL_Rect batch_rects[PAL_COUNT][BATCH_MAX];
static int      batch_count[PAL_COUNT];

/* RGB-значения палитры, в том же порядке, что и enum в render.h */
static const Uint8 pal_rgba[PAL_COUNT][4] = {
    {   0, 255, 255, 255 },   /* PAL_CYAN   */
    {   0,   0,   0, 255 },   /* PAL_BLACK  */
    { 255, 255, 255, 255 },   /* PAL_WHITE  */
    { 255,   0,   0, 255 },   /* PAL_RED    */
    {   0, 255,   0, 255 },   /* PAL_GREEN  */
    {   0, 100, 255, 255 },   /* PAL_BLUE   */
    { 255, 255,   0, 255 },   /* PAL_YELLOW */
    { 180,   0, 255, 255 },   /* PAL_PURPLE */
    { 128, 128, 128, 255 },   /* PAL_GRAY   */
    {   0, 200,   0, 255 }    /* PAL_WING   */
};

void batch_begin(void) {
    memset(batch_count, 0, sizeof(batch_count));
}

void batch_add(int pal, int x, int y, int w, int h) {
    if (pal < 0 || pal >= PAL_COUNT) return;
    if (batch_count[pal] >= BATCH_MAX) return;   /* переполнение - пропускаем */

    SDL_Rect* r = &batch_rects[pal][batch_count[pal]++];
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
}

void batch_flush(void) {
    for (int p = 0; p < PAL_COUNT; p++) {
        if (batch_count[p] == 0) continue;

        SDL_SetRenderDrawColor(renderer,
            pal_rgba[p][0], pal_rgba[p][1],
            pal_rgba[p][2], pal_rgba[p][3]);

        SDL_RenderFillRects(renderer, batch_rects[p], batch_count[p]);
    }
    batch_begin();
}


void init_font(void) {
    if (TTF_Init() == -1) {
        return;
    }

    font = TTF_OpenFont("arial.ttf", 28);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 28);
    }
    // Убираем все printf
}

void cleanup_font(void) {
    if (font) TTF_CloseFont(font);
    TTF_Quit();
}

void set_color(Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

void draw_rect(int x, int y, int w, int h, Color c) {
    set_color(c);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

/* Звёзды.
 * Было: 100 отдельных вызовов SDL_RenderDrawPoint за кадр.
 * Стало: 1 вызов SDL_RenderDrawPoints. */
static SDL_Point stars[100];
static bool stars_init = false;

void draw_stars(void) {
    if (!stars_init) {
        srand((unsigned int)time(NULL));
        for (int i = 0; i < 100; i++) {
            stars[i].x = rand() % SCREEN_WIDTH;
            stars[i].y = rand() % SCREEN_HEIGHT;
        }
        stars_init = true;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawPoints(renderer, stars, 100);
}

void draw_text(const char* text, int x, int y, Color c) {
    if (!font) {
        // Попробуем загрузить шрифт ещё раз, если он NULL
        font = TTF_OpenFont("arial.ttf", 28);
        if (!font) {
            font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 28);
        }
        if (!font) return;
    }

    SDL_Color color = { c.r, c.g, c.b, c.a };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}