#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    MENU_MAIN,
    MENU_LEADERBOARD,
    NAME_INPUT_STATE,
    DIFFICULTY_STATE,
    GAME_MODE_PLAYER_VS_AI,
    GAME_MODE_AI_VS_AI
} GameState;

typedef struct {
    int selected_option;
    bool leaderboard_shown;
} Menu;

extern GameState current_state;  
extern Menu menu;

void init_menu(void);
void handle_menu_input(void);
void draw_menu(void);
void draw_leaderboard(void);
void save_score(int score);
void load_leaderboard(void);
void save_leaderboard(void);
#endif