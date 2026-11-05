#define _CRT_SECURE_NO_WARNINGS

#include "name_input.h"
#include "render.h"
#include "game.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>

char player_name[MAX_NAME_LEN + 1] = { 0 };
bool name_input_active = false;

const char alphabet[ALPHABET_SIZE] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int selected_letter_index = 0;
int selected_position = 0;

void init_name_input(void) {
    memset(player_name, 0, sizeof(player_name));
    selected_letter_index = 0;
    selected_position = 0;
    name_input_active = true;
}

void reset_name_input(void) {
    memset(player_name, 0, sizeof(player_name));
    selected_letter_index = 0;
    selected_position = 0;
}

bool is_name_complete(void) {
    return (player_name[0] != '\0' && player_name[1] != '\0' && player_name[2] != '\0');
}

void handle_name_input(void) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    static int cooldown = 0;
    cooldown--;

    if (cooldown <= 0) {
        // ВЛЕВО/ВПРАВО - выбор буквы в алфавите
        if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
            selected_letter_index--;
            if (selected_letter_index < 0) selected_letter_index = ALPHABET_SIZE - 1;
            cooldown = 8;
        }
        if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
            selected_letter_index++;
            if (selected_letter_index >= ALPHABET_SIZE) selected_letter_index = 0;
            cooldown = 8;
        }

        // ВВЕРХ/ВНИЗ - перемещение по строкам алфавита (13 букв в строке)
        if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
            if (selected_letter_index >= 13) {
                selected_letter_index -= 13;
            }
            else {
                // Если в верхней строке, переходим в нижнюю
                if (selected_letter_index + 13 < ALPHABET_SIZE) {
                    selected_letter_index += 13;
                }
            }
            cooldown = 8;
        }
        if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
            if (selected_letter_index + 13 < ALPHABET_SIZE) {
                selected_letter_index += 13;
            }
            else {
                // Если в нижней строке, переходим в верхнюю
                if (selected_letter_index >= 13) {
                    selected_letter_index -= 13;
                }
            }
            cooldown = 8;
        }

        // ENTER или ПРОБЕЛ - выбор буквы (только если не все буквы введены)
        if ((state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_SPACE]) && selected_position < MAX_NAME_LEN) {
            if (selected_position < MAX_NAME_LEN) {
                player_name[selected_position] = alphabet[selected_letter_index];
                selected_position++;
                if (selected_position >= MAX_NAME_LEN) {
                    selected_position = MAX_NAME_LEN;
                }
            }
            cooldown = 10;
        }

        // BACKSPACE - удаление последней буквы
        if (state[SDL_SCANCODE_BACKSPACE] && selected_position > 0) {
            selected_position--;
            player_name[selected_position] = '\0';
            cooldown = 10;
        }

        // TAB или SHIFT - переключение между позициями букв
        if (state[SDL_SCANCODE_TAB] || state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
            if (selected_position > 0) {
                selected_position--;
                if (selected_position < 0) selected_position = 0;
                if (player_name[selected_position] != '\0') {
                    for (int i = 0; i < ALPHABET_SIZE; i++) {
                        if (alphabet[i] == player_name[selected_position]) {
                            selected_letter_index = i;
                            break;
                        }
                    }
                }
            }
            cooldown = 10;
        }
    }
}

void draw_name_input(void) {
    SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
    SDL_RenderClear(renderer);

    draw_stars();

    draw_text("ENTER YOUR NAME", SCREEN_WIDTH / 2 - 110, 60, COLOR_YELLOW);

    int name_box_width = 300;
    int name_box_x = SCREEN_WIDTH / 2 - name_box_width / 2;
    int name_box_y = 150;

    draw_rect(name_box_x - 10, name_box_y - 10, name_box_width + 20, 80, COLOR_BLUE);
    draw_rect(name_box_x - 5, name_box_y - 5, name_box_width + 10, 70, COLOR_BLACK);

    // Отображение 3 букв
    int letter_spacing = 90;
    int letter_start_x = name_box_x + (name_box_width - (letter_spacing * (MAX_NAME_LEN - 1) + 50)) / 2;

    for (int i = 0; i < MAX_NAME_LEN; i++) {
        int letter_x = letter_start_x + i * letter_spacing;
        int letter_y = name_box_y + 15;

        if (i == selected_position && selected_position < MAX_NAME_LEN) {
            draw_rect(letter_x - 5, letter_y - 5, 60, 50, COLOR_GREEN);
        }
        else {
            draw_rect(letter_x - 5, letter_y - 5, 60, 50, COLOR_BLUE);
        }

        char letter[2] = "_";
        if (player_name[i] != '\0') {
            letter[0] = player_name[i];
        }
        else {
            letter[0] = '_';
        }
        draw_text(letter, letter_x + 18, letter_y + 8, COLOR_WHITE);
    }

    // Алфавит - 2 строки по 13 букв
    int alphabet_start_y = 280;
    int alphabet_spacing_x = 55;
    int alphabet_spacing_y = 70;
    int letters_per_row = 13;
    int total_width = letters_per_row * alphabet_spacing_x;
    int alphabet_start_x = SCREEN_WIDTH / 2 - total_width / 2;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        int row = i / letters_per_row;
        int col = i % letters_per_row;
        int x = alphabet_start_x + col * alphabet_spacing_x;
        int y = alphabet_start_y + row * alphabet_spacing_y;

        char letter[2] = { alphabet[i], '\0' };

        if (i == selected_letter_index && selected_position < MAX_NAME_LEN) {
            draw_rect(x - 5, y - 5, 45, 45, COLOR_GREEN);
            draw_text(letter, x + 12, y + 8, COLOR_BLACK);
        }
        else {
            draw_rect(x - 5, y - 5, 45, 45, COLOR_BLUE);
            draw_text(letter, x + 12, y + 8, COLOR_WHITE);
        }
    }

    // Кнопка DELETE
    int delete_width = 200;
    int delete_x = SCREEN_WIDTH / 2 - delete_width / 2;
    int delete_y = 500;
    draw_rect(delete_x, delete_y, delete_width, 45, COLOR_RED);
    draw_text("DELETE", delete_x + 65, delete_y + 12, COLOR_WHITE);

    // Кнопка START
    if (is_name_complete()) {
        int start_width = 200;
        int start_x = SCREEN_WIDTH / 2 - start_width / 2;
        int start_y = 570;

        if (selected_position == MAX_NAME_LEN) {
            draw_rect(start_x, start_y, start_width, 45, COLOR_GREEN);
            draw_text("> START <", start_x + 55, start_y + 12, COLOR_BLACK);
        }
        else {
            draw_rect(start_x, start_y, start_width, 45, COLOR_BLUE);
            draw_text("START", start_x + 70, start_y + 12, COLOR_WHITE);
        }
    }

    // Убраны инструкции внизу

    SDL_RenderPresent(renderer);
}