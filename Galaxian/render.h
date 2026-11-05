#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    Uint8 r, g, b, a;
} Color;

#define COLOR_CYAN    (Color){0, 255, 255, 255}
#define COLOR_BLACK   (Color){0, 0, 0, 255}
#define COLOR_WHITE   (Color){255, 255, 255, 255}
#define COLOR_RED     (Color){255, 0, 0, 255}
#define COLOR_GREEN   (Color){0, 255, 0, 255}
#define COLOR_BLUE    (Color){0, 100, 255, 255}
#define COLOR_YELLOW  (Color){255, 255, 0, 255}
#define COLOR_PURPLE  (Color){180, 0, 255, 255}
#define COLOR_GRAY    (Color){128, 128, 128, 255}

extern SDL_Renderer* renderer;

void init_font(void);
void cleanup_font(void);
void set_color(Color c);
void draw_rect(int x, int y, int w, int h, Color c);
void draw_stars(void);
void draw_text(const char* text, int x, int y, Color c);

#endif