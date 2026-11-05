#ifndef NAME_INPUT_H
#define NAME_INPUT_H

#include <stdbool.h>
#include <SDL.h>
#define MAX_NAME_LEN 3
#define ALPHABET_SIZE 26
struct SDL_Renderer;
extern char player_name[MAX_NAME_LEN + 1];
extern bool name_input_active;  

void init_name_input(void);
void handle_name_input(void);
void draw_name_input(void);
bool is_name_complete(void);
void reset_name_input(void);

#endif