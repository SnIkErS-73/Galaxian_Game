#define _CRT_SECURE_NO_WARNINGS

#include "ai_player.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "game.h"
#include <stdlib.h>
#include <SDL.h>

static int ai_shoot_cooldown = 0;

// Находит ближайшего живого врага
int find_closest_enemy_ai(void) {
    int closest = -1;
    int min_distance = 10000;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            int distance = abs(enemies[i].x - player.x);
            if (distance < min_distance) {
                min_distance = distance;
                closest = i;
            }
        }
    }
    return closest;
}

void ai_move(void) {
    int target_x = SCREEN_WIDTH / 2;

    // Находим ближайшего врага
    int closest = find_closest_enemy_ai();
    if (closest != -1) {
        target_x = enemies[closest].x + enemies[closest].width / 2 - player.width / 2;
    }

    // Движение к цели
    if (player.x < target_x - 5 && player.x < SCREEN_WIDTH - player.width) {
        player.x += PLAYER_SPEED;
    }
    if (player.x > target_x + 5 && player.x > 0) {
        player.x -= PLAYER_SPEED;
    }
}

void ai_shoot(void) {
    ai_shoot_cooldown--;

    if (ai_shoot_cooldown <= 0) {
        int target = find_closest_enemy_ai();

        if (target != -1) {
            int dx = abs(enemies[target].x - player.x);
            if (dx < 60) {
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].x = player.x + player.width / 2 - 2;
                        bullets[i].y = player.y;
                        bullets[i].width = 4;
                        bullets[i].height = 15;
                        bullets[i].active = true;
                        bullets[i].is_enemy = false;
                        ai_shoot_cooldown = 12;
                        break;
                    }
                }
            }
        }
    }
}

void ai_update(void) {
    ai_move();
    ai_shoot();
}