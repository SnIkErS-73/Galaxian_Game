#define SDL_MAIN_HANDLED
#define _CRT_SECURE_NO_WARNINGS

#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "menu.h"
#include "render.h"
#include "name_input.h"
#include "difficulty.h"
#include "ai_player.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#ifdef main
#undef main
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand((unsigned int)time(NULL));

    if (!init_sdl()) {
        return 1;
    }

    init_font();
    init_menu();

    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        if (current_state == MENU_MAIN) {
            handle_menu_input();

            if (menu.leaderboard_shown) {
                draw_leaderboard();
            }
            else {
                draw_menu();
            }
        }
        else if (current_state == NAME_INPUT_STATE) {
            handle_name_input();
            draw_name_input();

            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (is_name_complete() && (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_SPACE])) {
                init_difficulty();
                current_state = DIFFICULTY_STATE;
            }
        }
        else if (current_state == DIFFICULTY_STATE) {
            handle_difficulty_input();
            draw_difficulty_menu();

            if (difficulty_selected) {
                current_state = GAME_MODE_PLAYER_VS_AI;
                game_over = false;
                game_won = false;
                score = 0;
                init_player();
                init_enemies();
                init_bullets();
            }
        }
        else if (current_state == GAME_MODE_PLAYER_VS_AI) {
            game_loop();
            if (game_over) {
                save_score(score);
                current_state = MENU_MAIN;
                menu.leaderboard_shown = false;
                game_over = false;
                game_won = false;
            }
        }
        else if (current_state == GAME_MODE_AI_VS_AI) {
            // AI ��������� �������
            ai_update();

            // ���������� ����
            update_bullets();
            update_enemies();
            check_collisions_ai();  // ����������� �������� ��� ����� � � �������������

            if (check_victory()) {
                game_over = true;
            }

            // ���������
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

            // ��������� ������
            draw_text("AI VS AI MODE", SCREEN_WIDTH / 2 - 70, 20, COLOR_YELLOW);

            SDL_RenderPresent(renderer);

            if (game_over) {
                current_state = MENU_MAIN;
                menu.leaderboard_shown = false;
                game_over = false;
                game_won = false;
            }

            SDL_Delay(16);
        }

        SDL_Delay(16);
    }

    cleanup_font();
    cleanup_sdl();
    return 0;
}