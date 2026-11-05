#include "enemy.h"
#include "bullet.h"
#include "player.h"
#include "game.h"
#include "difficulty.h"
#include <stdlib.h>

Enemy enemies[MAX_ENEMIES];

void init_enemies(void) {
    int count = 0;
    int enemy_count = get_enemy_count();

    // Определяем количество рядов и колонок в зависимости от сложности
    int rows, cols;
    switch (selected_difficulty) {
    case DIFFICULTY_EASY:
        rows = 3;
        cols = 4;
        break;
    case DIFFICULTY_NORMAL:
        rows = 4;
        cols = 5;
        break;
    case DIFFICULTY_HARD:
        rows = 4;
        cols = 8;
        break;
    default:
        rows = 4;
        cols = 5;
    }

    int spacing_x = SCREEN_WIDTH / (cols + 1);
    int spacing_y = 60;
    int start_x = spacing_x / 2;
    int start_y = 80;

    Color colors[] = { COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_PURPLE };
    int num_colors = sizeof(colors) / sizeof(Color);

    for (int row = 0; row < rows && count < MAX_ENEMIES; row++) {
        for (int col = 0; col < cols && count < MAX_ENEMIES; col++) {
            enemies[count].x = start_x + col * spacing_x;
            enemies[count].y = start_y + row * spacing_y;
            enemies[count].width = 40;
            enemies[count].height = 30;
            enemies[count].alive = true;
            enemies[count].diving = false;
            enemies[count].dive_step = 0;
            enemies[count].color = colors[count % num_colors];
            count++;
        }
    }
    for (int i = count; i < MAX_ENEMIES; i++) {
        enemies[i].alive = false;
    }
}

void update_enemies(void) {
    static int move_dir = 1;
    static int move_timer = 0;
    static int dive_timer = 0;
    move_timer++;
    dive_timer++;

    if (move_timer >= 30) {
        bool at_edge = false;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].alive && !enemies[i].diving) {
                if ((enemies[i].x <= 10 && move_dir == -1) ||
                    (enemies[i].x + enemies[i].width >= SCREEN_WIDTH - 10 && move_dir == 1)) {
                    at_edge = true;
                    break;
                }
            }
        }

        if (at_edge) {
            move_dir *= -1;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive && !enemies[i].diving) {
                    enemies[i].y += 20;
                    if (enemies[i].y + enemies[i].height >= PLAYER_Y) {
                        player.alive = false;
                        game_over = true;
                    }
                }
            }
        }
        else {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive && !enemies[i].diving) {
                    enemies[i].x += move_dir * get_enemy_speed();
                }
            }
        }
        move_timer = 0;
    }

    if (dive_timer >= 100 && !game_over) {
        int candidates = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].alive && !enemies[i].diving) candidates++;
        }
        if (candidates > 0) {
            int target = rand() % candidates;
            int count = 0;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive && !enemies[i].diving) {
                    if (count == target) {
                        enemies[i].diving = true;
                        enemies[i].dive_target_x = player.x + player.width / 2;
                        enemies[i].dive_step = 0;
                        break;
                    }
                    count++;
                }
            }
        }
        dive_timer = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive && enemies[i].diving) {
            enemies[i].dive_step++;

            if (enemies[i].dive_step < 30) {
                enemies[i].y += get_enemy_dive_speed();
                if (enemies[i].x < enemies[i].dive_target_x) enemies[i].x += 2;
                else if (enemies[i].x > enemies[i].dive_target_x) enemies[i].x -= 2;

                if (abs(enemies[i].x - player.x) < 40 && abs(enemies[i].y - player.y) < 30) {
                    player.alive = false;
                    game_over = true;
                }

                if (enemies[i].dive_step == 20) {
                    for (int b = 0; b < MAX_BULLETS; b++) {
                        if (!bullets[b].active) {
                            bullets[b].x = enemies[i].x + enemies[i].width / 2 - 2;
                            bullets[b].y = enemies[i].y + enemies[i].height;
                            bullets[b].width = 4;
                            bullets[b].height = 15;
                            bullets[b].active = true;
                            bullets[b].is_enemy = true;
                            break;
                        }
                    }
                }
            }
            else {
                if (enemies[i].y > 100) {
                    enemies[i].y -= 3;
                    if (enemies[i].x < enemies[i].dive_target_x) enemies[i].x -= 2;
                    else if (enemies[i].x > enemies[i].dive_target_x) enemies[i].x += 2;
                }
                else {
                    enemies[i].diving = false;
                }
            }
        }
    }
}

void draw_enemy(Enemy* e) {
    if (!e->alive) return;

    int cx = e->x + e->width / 2;
    int cy = e->y + e->height / 2;

    draw_rect(cx - 15, cy - 10, 30, 20, e->color);
    draw_rect(cx - 10, cy - 20, 20, 10, e->color);
    draw_rect(cx - 18, cy - 22, 4, 8, COLOR_WHITE);
    draw_rect(cx + 14, cy - 22, 4, 8, COLOR_WHITE);
}

bool check_victory(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) return false;
    }
    return true;
}