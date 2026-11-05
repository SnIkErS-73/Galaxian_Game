#define _CRT_SECURE_NO_WARNINGS

#include "difficulty.h"
#include "render.h"
#include "game.h"
#include <SDL.h>
#include <stdio.h>

Difficulty selected_difficulty = DIFFICULTY_NORMAL;
bool difficulty_selected = false;



void init_difficulty(void) {
    selected_difficulty = DIFFICULTY_NORMAL;
    difficulty_selected = false;
    printf("Difficulty initialized, waiting for input\n");
}

void handle_difficulty_input(void) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    static int cooldown = 0;
    cooldown--;

    if (cooldown <= 0) {
        if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A] ||
            state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
            if (selected_difficulty == DIFFICULTY_EASY) {
                selected_difficulty = DIFFICULTY_NORMAL;
            }
            else if (selected_difficulty == DIFFICULTY_NORMAL) {
                selected_difficulty = DIFFICULTY_HARD;
            }
            else {
                selected_difficulty = DIFFICULTY_EASY;
            }
            cooldown = 10;
        }
    }

    // ТОЛЬКО Enter, без Пробела (чтобы не конфликтовать)
    if (state[SDL_SCANCODE_RETURN]) {
        printf("Difficulty selected: %d\n", selected_difficulty);
        difficulty_selected = true;
        apply_difficulty_settings();
    }
}

void draw_difficulty_menu(void) {
    SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
    SDL_RenderClear(renderer);

    draw_stars();

    // Заголовок
    draw_text("SELECT DIFFICULTY", SCREEN_WIDTH / 2 - 120, 80, COLOR_YELLOW);

    // Три варианта сложности
    int button_width = 300;
    int button_height = 60;
    int button_x = SCREEN_WIDTH / 2 - button_width / 2;
    int button_y_start = 200;
    int spacing = 80;

    // EASY
    if (selected_difficulty == DIFFICULTY_EASY) {
        draw_rect(button_x, button_y_start, button_width, button_height, COLOR_GREEN);
        draw_text("> EASY <", button_x + 110, button_y_start + 20, COLOR_BLACK);
    }
    else {
        draw_rect(button_x, button_y_start, button_width, button_height, COLOR_BLUE);
        draw_text("EASY", button_x + 130, button_y_start + 20, COLOR_WHITE);
    }
    // NORMAL
    int button2_y = button_y_start + spacing + 40;
    if (selected_difficulty == DIFFICULTY_NORMAL) {
        draw_rect(button_x, button2_y, button_width, button_height, COLOR_GREEN);
        draw_text("> NORMAL <", button_x + 105, button2_y + 20, COLOR_BLACK);
    }
    else {
        draw_rect(button_x, button2_y, button_width, button_height, COLOR_YELLOW);
        draw_text("NORMAL", button_x + 120, button2_y + 20, COLOR_BLACK);
    }
    // HARD
    int button3_y = button2_y + spacing + 40;
    if (selected_difficulty == DIFFICULTY_HARD) {
        draw_rect(button_x, button3_y, button_width, button_height, COLOR_GREEN);
        draw_text("> HARD <", button_x + 110, button3_y + 20, COLOR_BLACK);
    }
    else {
        draw_rect(button_x, button3_y, button_width, button_height, COLOR_RED);
        draw_text("HARD", button_x + 130, button3_y + 20, COLOR_WHITE);
    }
    SDL_RenderPresent(renderer);
}

void apply_difficulty_settings(void) {
    switch (selected_difficulty) {
    case DIFFICULTY_EASY:
        // Количество врагов: 12 (3 ряда по 4)
        // Скорость: ENEMY_SPEED = 1, ENEMY_DIVE_SPEED = 2
        break;
    case DIFFICULTY_NORMAL:
        // Количество врагов: 20 (4 ряда по 5)
        // Скорость: ENEMY_SPEED = 2, ENEMY_DIVE_SPEED = 4
        break;
    case DIFFICULTY_HARD:
        // Количество врагов: 32 (4 ряда по 8)
        // Скорость: ENEMY_SPEED = 3, ENEMY_DIVE_SPEED = 6
        break;
    }
}

int get_score_multiplier(void) {
    switch (selected_difficulty) {
    case DIFFICULTY_EASY:   return 1;
    case DIFFICULTY_NORMAL: return 2;
    case DIFFICULTY_HARD:   return 3;
    default:                return 1;
    }
}

int get_enemy_count(void) {
    switch (selected_difficulty) {
    case DIFFICULTY_EASY:   return 12;
    case DIFFICULTY_NORMAL: return 20;
    case DIFFICULTY_HARD:   return 32;
    default:                return 20;
    }
}

int get_enemy_speed(void) {
    switch (selected_difficulty) {
    case DIFFICULTY_EASY:   return 1;
    case DIFFICULTY_NORMAL: return 2;
    case DIFFICULTY_HARD:   return 3;
    default:                return 2;
    }
}

int get_enemy_dive_speed(void) {
    switch (selected_difficulty) {
    case DIFFICULTY_EASY:   return 3;
    case DIFFICULTY_NORMAL: return 5;
    case DIFFICULTY_HARD:   return 7;
    default:                return 5;
    }
}