#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define SCREEN_WIDTH  1024   
#define SCREEN_HEIGHT 768    
#define PLAYER_Y      (SCREEN_HEIGHT - 60)
#define MAX_BULLETS   10
#define MAX_ENEMIES   20
#define PLAYER_SPEED  8
#define BULLET_SPEED  10
#define ENEMY_SPEED   2
#define ENEMY_DIVE_SPEED 4

extern int score;
extern bool game_over;
extern bool game_won;
extern bool return_to_menu;

bool init_sdl(void);
void cleanup_sdl(void);
void game_loop(void);
void check_collisions_ai(void);
#endif