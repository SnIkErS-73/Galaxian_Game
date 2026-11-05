#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <stdbool.h>

typedef enum {
    DIFFICULTY_EASY,
    DIFFICULTY_NORMAL,
    DIFFICULTY_HARD
} Difficulty;

extern Difficulty selected_difficulty;
extern bool difficulty_selected;

void init_difficulty(void);
void handle_difficulty_input(void);
void draw_difficulty_menu(void);
void apply_difficulty_settings(void);
int get_score_multiplier(void);
int get_enemy_count(void);
int get_enemy_speed(void);
int get_enemy_dive_speed(void);

#endif