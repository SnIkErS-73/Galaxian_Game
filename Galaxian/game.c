#define _CRT_SECURE_NO_WARNINGS

#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "render.h"
#include "menu.h"
#include "difficulty.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int score = 0;
bool game_over = false;
bool game_won = false;
bool return_to_menu = false;  // Флаг для выхода в меню

SDL_Window* window = NULL;
const Uint32 FRAME_DELAY = 16;

// Инициализация SDL
bool init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    window = SDL_CreateWindow(
        "Galaxian",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return true;
}

// Очистка ресурсов
void cleanup_sdl() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

// Обработка ввода
void handle_input() {
    const Uint8* state = SDL_GetKeyboardState(NULL);

    // Движение игрока
    if (state[SDL_SCANCODE_LEFT] && player.x > 0) {
        player.x -= PLAYER_SPEED;
    }
    if (state[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - player.width) {
        player.x += PLAYER_SPEED;
    }

    // Выстрел
    static int shoot_cooldown = 0;
    shoot_cooldown--;
    if (state[SDL_SCANCODE_SPACE] && shoot_cooldown <= 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].x = player.x + player.width / 2 - 2;
                bullets[i].y = player.y;
                bullets[i].width = 4;
                bullets[i].height = 15;
                bullets[i].active = true;
                bullets[i].is_enemy = false;
                shoot_cooldown = 15;
                break;
            }
        }
    }

    // ESC - выход в меню во время игры
    if (state[SDL_SCANCODE_ESCAPE]) {
        return_to_menu = true;
        game_over = true;
    }
}

// Проверка столкновений
void check_collisions() {
    // Снаряды игрока vs враги
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && !bullets[b].is_enemy) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (enemies[e].alive) {
                    if (bullets[b].x < enemies[e].x + enemies[e].width &&
                        bullets[b].x + bullets[b].width > enemies[e].x &&
                        bullets[b].y < enemies[e].y + enemies[e].height &&
                        bullets[b].y + bullets[b].height > enemies[e].y) {
                        enemies[e].alive = false;
                        bullets[b].active = false;
                        score += 100 * get_score_multiplier();
                        break;
                    }
                }
            }
        }
    }
    // Снаряды врагов vs игрок
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && bullets[b].is_enemy) {
            if (bullets[b].x < player.x + player.width &&
                bullets[b].x + bullets[b].width > player.x &&
                bullets[b].y < player.y + player.height &&
                bullets[b].y + bullets[b].height > player.y) {
                player.alive = false;
                game_over = true;
                bullets[b].active = false;
            }
        }
    }
}

// Основной игровой цикл
void game_loop() {
    SDL_Event event;
    Uint32 frame_start;
    int frame_time;

    // Сбрасываем флаги перед началом игры
    game_over = false;
    game_won = false;
    return_to_menu = false;
    score = 0;
    init_player();
    init_enemies();
    init_bullets();

    // Игровой цикл
    if (current_state == GAME_MODE_AI_VS_AI) {
        return;
    }
    while (!game_over && !return_to_menu) {
        frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game_over = true;
                return;
            }
        }

        handle_input();
        if (game_over) break;

        update_bullets();
        update_enemies();
        check_collisions();

        if (check_victory()) {
            game_won = true;
            game_over = true;
        }

        // Отрисовка
        SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
        SDL_RenderClear(renderer);

        draw_stars();

        for (int i = 0; i < MAX_BULLETS; i++) {
            draw_bullet(&bullets[i]);
        }
        for (int i = 0; i < MAX_ENEMIES; i++) {
            draw_enemy(&enemies[i]);
        }
        draw_player();

        SDL_RenderPresent(renderer);

        frame_time = SDL_GetTicks() - frame_start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    // Если вышли в меню по ESC - просто возвращаемся
    if (return_to_menu) {
        current_state = MENU_MAIN;
        return;
    }

    // ЭКРАН ОКОНЧАНИЯ ИГРЫ С ВЫБОРОМ
    int selected_option = 0;  // 0 - Replay, 1 - Menu
    bool choosing = true;
    Uint32 last_key_time = 0;

    while (choosing) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                choosing = false;
                return;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        Uint32 current_time = SDL_GetTicks();

        // Навигация по кнопкам
        if (current_time - last_key_time > 150) {
            if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN] ||
                state[SDL_SCANCODE_W] || state[SDL_SCANCODE_S]) {
                selected_option = 1 - selected_option;
                last_key_time = current_time;
            }
        }

        // Выбор
        if (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_SPACE]) {
            if (selected_option == 0) {
                // REPLAY - перезапускаем игру
                choosing = false;
                game_loop();
                return;
            }
            else {
                // MENU - выходим в главное меню
                choosing = false;
                current_state = MENU_MAIN;
                return;
            }
        }

        // ESC - выход в меню
        if (state[SDL_SCANCODE_ESCAPE]) {
            choosing = false;
            current_state = MENU_MAIN;
            return;
        }

        // Отрисовка экрана результатов
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_stars();

        // Полупрозрачный фон
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &overlay);

        // Рамка
        draw_rect(SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 - 120, 360, 240, COLOR_BLUE);
        draw_rect(SCREEN_WIDTH / 2 - 175, SCREEN_HEIGHT / 2 - 115, 350, 230, COLOR_BLACK);

        // Заголовок
        if (game_won) {
            draw_text("VICTORY!", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 90, COLOR_YELLOW);
        }
        else {
            draw_text("GAME OVER", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 90, COLOR_RED);
        }

        // Счёт
        char score_text[50];
        sprintf(score_text, "SCORE: %d", score);
        draw_text(score_text, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 40, COLOR_WHITE);

        // Кнопка REPLAY
        int button_width = 200;
        int button_height = 40;
        int button_x = SCREEN_WIDTH / 2 - button_width / 2;
        int button_y = SCREEN_HEIGHT / 2 + 10;

        if (selected_option == 0) {
            draw_rect(button_x, button_y, button_width, button_height, COLOR_GREEN);
            draw_text("> REPLAY <", button_x + 55, button_y + 10, COLOR_BLACK);
        }
        else {
            draw_rect(button_x, button_y, button_width, button_height, COLOR_BLUE);
            draw_text("REPLAY", button_x + 70, button_y + 10, COLOR_WHITE);
        }

        // Кнопка MENU
        int button2_y = button_y + 55;

        if (selected_option == 1) {
            draw_rect(button_x, button2_y, button_width, button_height, COLOR_GREEN);
            draw_text("> MENU <", button_x + 65, button2_y + 10, COLOR_BLACK);
        }
        else {
            draw_rect(button_x, button2_y, button_width, button_height, COLOR_BLUE);
            draw_text("MENU", button_x + 80, button2_y + 10, COLOR_WHITE);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
void check_collisions_ai(void) {
    // Только пули игрока уничтожают врагов (без начисления очков)
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && !bullets[b].is_enemy) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (enemies[e].alive) {
                    if (bullets[b].x < enemies[e].x + enemies[e].width &&
                        bullets[b].x + bullets[b].width > enemies[e].x &&
                        bullets[b].y < enemies[e].y + enemies[e].height &&
                        bullets[b].y + bullets[b].height > enemies[e].y) {
                        enemies[e].alive = false;
                        bullets[b].active = false;
                        break;
                    }
                }
            }
        }
    }
    // Игрок неуязвим - проверку попадания в игрока НЕ делаем
}