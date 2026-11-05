#include "player.h"
#include "render.h"
#include "game.h"

Player player;

void init_player(void) {
    player.x = SCREEN_WIDTH / 2 - 25;
    player.y = PLAYER_Y;
    player.width = 50;
    player.height = 30;
    player.alive = true;
}

void draw_player(void) {
    if (!player.alive) return;

    Color body = COLOR_GREEN;
    Color wing = { 0, 200, 0, 255 };

    draw_rect(player.x + 10, player.y, 30, 25, body);
    draw_rect(player.x, player.y + 15, 10, 10, wing);
    draw_rect(player.x + 40, player.y + 15, 10, 10, wing);
    draw_rect(player.x + 23, player.y - 8, 14, 10, body);
}