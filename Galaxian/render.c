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

void draw_stars(void) {
    static int stars[100][2];
    static bool stars_init = false;

    if (!stars_init) {
        srand((unsigned int)time(NULL));
        for (int i = 0; i < 100; i++) {
            stars[i][0] = rand() % SCREEN_WIDTH;
            stars[i][1] = rand() % SCREEN_HEIGHT;
        }
        stars_init = true;
    }

    set_color(COLOR_WHITE);
    for (int i = 0; i < 100; i++) {
        SDL_RenderDrawPoint(renderer, stars[i][0], stars[i][1]);
    }
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