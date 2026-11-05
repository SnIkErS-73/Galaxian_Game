#define _CRT_SECURE_NO_WARNINGS
#define LEADERBOARD_FILE "leaderboard.dat"
#include "menu.h"
#include "render.h"
#include "game.h"
#include "name_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

GameState current_state = MENU_MAIN;
Menu menu;

#define MAX_SCORES 10
#define NAME_LEN 20

typedef struct {
    char name[NAME_LEN];
    int score;
} LeaderboardEntry;

LeaderboardEntry leaderboard[MAX_SCORES];
int num_scores = 0;

void init_menu(void) {
    menu.selected_option = 0;
    menu.leaderboard_shown = false;
    load_leaderboard();
}

void draw_menu(void) {
    SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
    SDL_RenderClear(renderer);

    draw_stars();

    // Название игры
    draw_text("GALAXIAN", SCREEN_WIDTH / 2 - 90, 80, COLOR_YELLOW);

    int button_width = 350;
    int button_height = 60;
    int button_x = SCREEN_WIDTH / 2 - button_width / 2;
    int button_y_start = 200;
    int spacing = 80;

    // Кнопка "P1 vs AI"
    if (menu.selected_option == 0) {
        draw_rect(button_x, button_y_start, button_width, button_height, COLOR_GREEN);
        draw_text("> P1 VS AI <", button_x + 110, button_y_start + 20, COLOR_BLACK);
    }
    else {
        draw_rect(button_x, button_y_start, button_width, button_height, COLOR_BLUE);
        draw_text("P1 VS AI", button_x + 130, button_y_start + 20, COLOR_WHITE);
    }

    // Кнопка "AI vs AI"
    int button2_y = button_y_start + spacing;
    if (menu.selected_option == 1) {
        draw_rect(button_x, button2_y, button_width, button_height, COLOR_GRAY);
        draw_text("> AI VS AI <", button_x + 110, button2_y + 20, COLOR_BLACK);
    }
    else if (menu.selected_option == 1) {
        printf("AI vs AI mode selected\n");
        current_state = GAME_MODE_AI_VS_AI;
        game_over = false;
        game_won = false;
        score = 0;
        init_player();
        init_enemies();
        init_bullets();
    }
    else {
        draw_rect(button_x, button2_y, button_width, button_height, COLOR_BLUE);
        draw_text("AI VS AI", button_x + 130, button2_y + 20, COLOR_WHITE);
    }

    // Кнопка "Таблица лидеров"
    int button3_y = button2_y + spacing;
    if (menu.selected_option == 2) {
        draw_rect(button_x, button3_y, button_width, button_height, COLOR_PURPLE);
        draw_text("> LEADERBOARD <", button_x + 95, button3_y + 20, COLOR_BLACK);
    }
    else {
        draw_rect(button_x, button3_y, button_width, button_height, COLOR_BLUE);
        draw_text("LEADERBOARD", button_x + 115, button3_y + 20, COLOR_WHITE);
    }

    SDL_RenderPresent(renderer);
}

void draw_leaderboard(void) {
    // Очищаем экран и рисуем звёзды
    SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
    SDL_RenderClear(renderer);

    // Рисуем звёзды
    draw_stars();

    // Полупрозрачный оверлей (чтобы таблица была читаемой)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &overlay);

    // Заголовок
    draw_rect(SCREEN_WIDTH / 2 - 180, 30, 360, 50, COLOR_PURPLE);
    draw_text("=== LEADERBOARD ===", SCREEN_WIDTH / 2 - 110, 48, COLOR_YELLOW);

    // Шапка таблицы
    int start_y = 110;
    int col1_x = SCREEN_WIDTH / 2 - 250;
    int col2_x = SCREEN_WIDTH / 2 - 120;
    int col3_x = SCREEN_WIDTH / 2 + 100;

    draw_text("#", col1_x, start_y, COLOR_CYAN);
    draw_text("NAME", col2_x, start_y, COLOR_CYAN);
    draw_text("SCORE", col3_x, start_y, COLOR_CYAN);

    // Линия под шапкой
    draw_rect(SCREEN_WIDTH / 2 - 280, start_y + 25, 560, 2, COLOR_CYAN);

    // Вывод записей
    for (int i = 0; i < num_scores && i < 10; i++) {
        int y = start_y + 50 + i * 40;

        // Номер
        char num_str[10];
        sprintf(num_str, "%d.", i + 1);
        draw_text(num_str, col1_x, y, COLOR_WHITE);

        // Имя
        draw_text(leaderboard[i].name, col2_x, y, COLOR_WHITE);

        // Счёт
        char score_str[20];
        sprintf(score_str, "%d", leaderboard[i].score);

        if (i == 0) {
            draw_text(score_str, col3_x, y, COLOR_YELLOW);
        }
        else if (i == 1) {
            draw_text(score_str, col3_x, y, COLOR_GRAY);
        }
        else if (i == 2) {
            draw_text(score_str, col3_x, y, COLOR_RED);
        }
        else {
            draw_text(score_str, col3_x, y, COLOR_GREEN);
        }

        // Разделительная линия
        draw_rect(SCREEN_WIDTH / 2 - 280, y + 30, 560, 1, COLOR_GRAY);
    }

    // Если нет записей
    if (num_scores == 0) {
        draw_text("No scores yet!", SCREEN_WIDTH / 2 - 60, 250, COLOR_GRAY);
    }

    // Кнопка возврата
    draw_rect(SCREEN_WIDTH / 2 - 100, 620, 200, 40, COLOR_RED);
    draw_text("ESC - BACK", SCREEN_WIDTH / 2 - 65, 632, COLOR_WHITE);

    SDL_RenderPresent(renderer);
}

void handle_menu_input(void) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    static int cooldown = 0;
    cooldown--;

    if (cooldown <= 0) {
        if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
            menu.selected_option--;
            if (menu.selected_option < 0) menu.selected_option = 2;
            cooldown = 10;
            printf("Selected: %d\n", menu.selected_option);  // Отладка
        }
        if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
            menu.selected_option++;
            if (menu.selected_option > 2) menu.selected_option = 0;
            cooldown = 10;
            printf("Selected: %d\n", menu.selected_option);  // Отладка
        }
    }

    if (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_SPACE]) {
        printf("Pressed ENTER! Option: %d\n", menu.selected_option);  // Отладка
        if (menu.selected_option == 0) {
            printf("Starting name input...\n");
            init_name_input();
            current_state = NAME_INPUT_STATE;
        }
        else if (menu.selected_option == 1) {
            printf("AI vs AI mode selected\n");
            current_state = GAME_MODE_AI_VS_AI;
            game_over = false;
            game_won = false;
            score = 0;
            init_player();
            init_enemies();
            init_bullets();
        }
        else if (menu.selected_option == 2) {
            printf("Showing leaderboard\n");
            menu.leaderboard_shown = true;
        }
    }

    if (state[SDL_SCANCODE_ESCAPE]) {
        if (menu.leaderboard_shown) {
            menu.leaderboard_shown = false;
        }
    }
}

void save_score(int score) {
    // Добавляем новый результат с именем игрока
    if (num_scores < MAX_SCORES) {
        strcpy(leaderboard[num_scores].name, player_name);
        leaderboard[num_scores].score = score;
        num_scores++;
    }
    else {
        // Находим самый низкий счёт
        int min_index = 0;
        for (int i = 1; i < MAX_SCORES; i++) {
            if (leaderboard[i].score < leaderboard[min_index].score) {
                min_index = i;
            }
        }
        // Заменяем, если текущий счёт выше
        if (score > leaderboard[min_index].score) {
            strcpy(leaderboard[min_index].name, player_name);
            leaderboard[min_index].score = score;
        }
    }

    // Сортировка по убыванию
    for (int i = 0; i < num_scores - 1; i++) {
        for (int j = i + 1; j < num_scores; j++) {
            if (leaderboard[i].score < leaderboard[j].score) {
                LeaderboardEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }

    // Ограничиваем до 10 записей
    if (num_scores > MAX_SCORES) {
        num_scores = MAX_SCORES;
    }

    // СОХРАНЯЕМ В ФАЙЛ
    save_leaderboard();
}

void load_leaderboard(void) {
    FILE* file = fopen(LEADERBOARD_FILE, "rb");

    if (file) {
        // Читаем количество записей
        fread(&num_scores, sizeof(int), 1, file);

        // Ограничиваем до MAX_SCORES
        if (num_scores > MAX_SCORES) {
            num_scores = MAX_SCORES;
        }

        // Читаем записи
        fread(leaderboard, sizeof(LeaderboardEntry), num_scores, file);

        fclose(file);
        printf("Loaded %d scores from file\n", num_scores);
    }
    else {
        // Если файла нет - создаём тестовые данные
        printf("No save file found, creating default scores\n");
        num_scores = 5;
        sprintf(leaderboard[0].name, "MASTER");
        leaderboard[0].score = 5000;
        sprintf(leaderboard[1].name, "WARRIOR");
        leaderboard[1].score = 3200;
        sprintf(leaderboard[2].name, "KNIGHT");
        leaderboard[2].score = 2100;
        sprintf(leaderboard[3].name, "ROOKIE");
        leaderboard[3].score = 1200;
        sprintf(leaderboard[4].name, "NOOB");
        leaderboard[4].score = 500;

        // Сохраняем тестовые данные
        save_leaderboard();
    }
}

void save_leaderboard(void) {
    FILE* file = fopen(LEADERBOARD_FILE, "wb");

    if (file) {
        // Сохраняем количество записей
        fwrite(&num_scores, sizeof(int), 1, file);

        // Сохраняем записи
        fwrite(leaderboard, sizeof(LeaderboardEntry), num_scores, file);

        fclose(file);
        printf("Saved %d scores to file\n", num_scores);
    }
    else {
        printf("Error saving leaderboard!\n");
    }
}