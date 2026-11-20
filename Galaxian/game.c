#define _CRT_SECURE_NO_WARNINGS

#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "render.h"
#include "menu.h"
#include "difficulty.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/* Разбивка времени отрисовки по фазам - для поиска настоящего узкого места */
static double t_clear = 0, t_stars = 0, t_fill = 0, t_present = 0;


/* 0 - новая отрисовка (батчинг), 1 - старая (как в исходной курсовой).
 * Нужна, чтобы в одном прогоне сравнить оба варианта на одних данных. */
static int bench_old_draw = 0;


int score = 0;
bool game_over = false;
bool game_won = false;
bool return_to_menu = false;  // Флаг для выхода в меню

SDL_Window* window = NULL;
const Uint32 FRAME_DELAY = 16;

// Инициализация SDL
bool init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    window = SDL_CreateWindow(
        "Galaxian",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return true;
}

// Очистка ресурсов
void cleanup_sdl() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

// Обработка ввода
void handle_input() {
    const Uint8* state = SDL_GetKeyboardState(NULL);

    // Движение игрока
    if (state[SDL_SCANCODE_LEFT] && player.x > 0) {
        player.x -= PLAYER_SPEED;
    }
    if (state[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - player.width) {
        player.x += PLAYER_SPEED;
    }

    // Выстрел
    static int shoot_cooldown = 0;
    shoot_cooldown--;
    if (state[SDL_SCANCODE_SPACE] && shoot_cooldown <= 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].x = player.x + player.width / 2 - 2;
                bullets[i].y = player.y;
                bullets[i].width = 4;
                bullets[i].height = 15;
                bullets[i].active = true;
                bullets[i].is_enemy = false;
                shoot_cooldown = 15;
                break;
            }
        }
    }

    // ESC - выход в меню во время игры
    if (state[SDL_SCANCODE_ESCAPE]) {
        return_to_menu = true;
        game_over = true;
    }
}

// Проверка столкновений
void check_collisions() {
    // Снаряды игрока vs враги
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && !bullets[b].is_enemy) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (enemies[e].alive) {
                    if (bullets[b].x < enemies[e].x + enemies[e].width &&
                        bullets[b].x + bullets[b].width > enemies[e].x &&
                        bullets[b].y < enemies[e].y + enemies[e].height &&
                        bullets[b].y + bullets[b].height > enemies[e].y) {
                        enemies[e].alive = false;
                        bullets[b].active = false;
                        score += 100 * get_score_multiplier();
                        break;
                    }
                }
            }
        }
    }
    // Снаряды врагов vs игрок
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && bullets[b].is_enemy) {
            if (bullets[b].x < player.x + player.width &&
                bullets[b].x + bullets[b].width > player.x &&
                bullets[b].y < player.y + player.height &&
                bullets[b].y + bullets[b].height > player.y) {
                player.alive = false;
                game_over = true;
                bullets[b].active = false;
            }
        }
    }
}



/* Текущее время в микросекундах (точнее, чем SDL_GetTicks) */
static double bench_us(void) {
    static Uint64 freq = 0;
    static Uint64 c = 0;

    /* В SDL2 2.0.18+ обе функции возвращают значение
     * и не принимают аргументов. */
    if (freq == 0) {
        freq = SDL_GetPerformanceFrequency();
        if (freq == 0) return 0.0;
    }

    c = SDL_GetPerformanceCounter();
    return (double)c * 1000000.0 / (double)freq;
}




/* ---------------------------------------------------------------------
 * СТАРАЯ отрисовка - копия исходного кода курсовой.
 * Используется только в бенчмарке как точка отсчёта.
 * --------------------------------------------------------------------- */
static void draw_enemy_old(Enemy* e) {
    if (!e->alive) return;

    int cx = e->x + e->width / 2;
    int cy = e->y + e->height / 2;

    draw_rect(cx - 15, cy - 10, 30, 20, e->color);
    draw_rect(cx - 10, cy - 20, 20, 10, e->color);
    draw_rect(cx - 18, cy - 22, 4, 8, COLOR_WHITE);
    draw_rect(cx + 14, cy - 22, 4, 8, COLOR_WHITE);
}

static void draw_bullet_old(Bullet* b) {
    if (!b->active) return;
    draw_rect(b->x, b->y, b->width, b->height, b->is_enemy ? COLOR_RED : COLOR_YELLOW);
}

static void draw_player_old(void) {
    if (!player.alive) return;

    Color body = COLOR_GREEN;
    Color wing = { 0, 200, 0, 255 };

    draw_rect(player.x + 10, player.y, 30, 25, body);
    draw_rect(player.x, player.y + 15, 10, 10, wing);
    draw_rect(player.x + 40, player.y + 15, 10, 10, wing);
    draw_rect(player.x + 23, player.y - 8, 14, 10, body);
}



/* Единая функция отрисовки игрового кадра.
 * Используется и в обычной игре, и в бенчмарке — поэтому бенчарк
 * всегда меряет ровно тот же код, что и игра. */
static void draw_game_frame(void) {
    double a = bench_us();

    SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
    SDL_RenderClear(renderer);

    double b = bench_us();
    draw_stars();
    double c = bench_us();

    if (bench_old_draw) {
        /* СТАРАЯ отрисовка - как в исходной версии курсовой */
        for (int i = 0; i < MAX_BULLETS; i++) {
            draw_bullet_old(&bullets[i]);
        }
        for (int i = 0; i < MAX_ENEMIES; i++) {
            draw_enemy_old(&enemies[i]);
        }
        draw_player_old();
    }
    else {
        /* НОВАЯ отрисовка - батчинг по цветам */
        batch_begin();
        for (int i = 0; i < MAX_BULLETS; i++) {
            draw_bullet(&bullets[i]);
        }
        for (int i = 0; i < MAX_ENEMIES; i++) {
            draw_enemy(&enemies[i]);
        }
        draw_player();
        batch_flush();
    }

    double d = bench_us();
    SDL_RenderPresent(renderer);
    double e = bench_us();

    t_clear += (b - a);
    t_stars += (c - b);
    t_fill += (d - c);
    t_present += (e - d);
}



/* =====================================================================
 * РЕЖИМ БЕНЧМАРКА
 *
 * Запускается клавишей B в главном меню.
 *
 * Ключевое свойство: каждый кадр выполняет РОВНО ОДИНАКОВУЮ нагрузку
 *   - одинаковое число живых врагов (убитые восстанавливаются),
 *   - одинаковое число пуль в полёте,
 *   - фиксированный seed rand() -> одинаковая последовательность.
 * Поэтому результат воспроизводим от запуска к запуску, и результаты
 * "до" и "после" можно сравнивать напрямую.
 *
 * Отдельно меряется время обновления и время отрисовки - это и есть
 * два разных узких места.
 * ===================================================================== */

#define BENCH_TARGET_BULLETS 32



 /* ---------------------------------------------------------------------
  * ПОДГОТОВКА СЦЕНАРИЯ
  * Сбрасывает состояние игры в одинаковое начальное положение, чтобы
  * оба прогона (старая и новая отрисовка) выполняли РОВНО одинаковую
  * работу и сравнение было честным.
  * --------------------------------------------------------------------- */
static int bench_orig_x[MAX_ENEMIES];
static int bench_orig_y[MAX_ENEMIES];
static int bench_spawned = 0;

static void bench_setup(void) {
    /* Одинаковая последовательность rand() в каждом прогоне */
    srand(12345);

    selected_difficulty = DIFFICULTY_HARD;

    init_player();
    init_enemies();
    init_bullets();

    bench_spawned = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        bench_orig_x[i] = enemies[i].x;
        bench_orig_y[i] = enemies[i].y;
        if (enemies[i].alive) bench_spawned++;
    }
}

/* ---------------------------------------------------------------------
 * ОДИН ПРОГОН: frames кадров.
 *   update_ms  - обновление логики
 *   fill_ms    - отрисовка объектов (без ожидания vsync)
 *   present_ms - SDL_RenderPresent (ожидание вертикальной синхронизации)
 * --------------------------------------------------------------------- */
static void bench_run_frames(int frames, double* update_ms,
    double* fill_ms, double* present_ms) {
    double upd = 0.0;
    t_clear = t_stars = t_fill = t_present = 0;

    for (int f = 0; f < frames; f++) {
        /* ---------------- ОБНОВЛЕНИЕ ---------------- */
        double t0 = bench_us();

        update_bullets();
        update_enemies();
        check_collisions();

        /* Сбрасываем игровые флаги, чтобы сценарий не прерывался */
        game_over = false;
        game_won = false;
        player.alive = true;

        /* Восстанавливаем убитых врагов -> постоянное число живых.
         * ТОЛЬКО реально созданных, иначе "воскреснут" пустые слоты. */
        for (int i = 0; i < bench_spawned; i++) {
            if (!enemies[i].alive) {
                enemies[i].alive = true;
                enemies[i].diving = false;
                enemies[i].dive_step = 0;
                enemies[i].x = bench_orig_x[i];
                enemies[i].y = bench_orig_y[i];
            }
        }

        /* Поддерживаем постоянное число пуль в полёте */
        int active = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) active++;
        }
        for (int i = 0; i < MAX_BULLETS && active < BENCH_TARGET_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].x = 120 + (i * 37) % 700;
                bullets[i].y = 200 + (i * 53) % 300;
                bullets[i].width = 4;
                bullets[i].height = 15;
                bullets[i].active = true;
                bullets[i].is_enemy = (i & 1);
                active++;
            }
        }

        double t1 = bench_us();

        /* ---------------- ОТРИСОВКА ---------------- */
        draw_game_frame();

        upd += (t1 - t0);
    }

    *update_ms = upd / frames / 1000.0;
    *fill_ms = t_fill / frames / 1000.0;
    *present_ms = t_present / frames / 1000.0;
}

void run_benchmark(int frames) {
    Difficulty saved_difficulty = selected_difficulty;

    double old_upd, old_fill, old_present;
    double new_upd, new_fill, new_present;

    /* ---- ПРОГОН 1: старая отрисовка (точка отсчёта) ---- */
    bench_setup();
    bench_old_draw = 1;
    bench_run_frames(frames, &old_upd, &old_fill, &old_present);

    /* ---- ПРОГОН 2: новая отрисовка (батчинг) ---- */
    bench_setup();
    bench_old_draw = 0;
    bench_run_frames(frames, &new_upd, &new_fill, &new_present);

    bench_old_draw = 0;
    selected_difficulty = saved_difficulty;

    FILE* f = fopen("C:/Users/OlegL/Desktop/Galaxian_github/bench_log.txt", "a");
    if (f) {
        fprintf(f, "enemies=%d bullets=%d frames=%d\n",
            bench_spawned, BENCH_TARGET_BULLETS, frames);
        fprintf(f, "  OLD draw: update=%.4f fill=%.4f work=%.4f present=%.4f ms\n",
            old_upd, old_fill, old_upd + old_fill, old_present);
        fprintf(f, "  NEW draw: update=%.4f fill=%.4f work=%.4f present=%.4f ms  fill_speedup=%.2fx\n",
            new_upd, new_fill, new_upd + new_fill, new_present,
            (new_fill > 0.0) ? (old_fill / new_fill) : 0.0);
        fclose(f);
    }
}



// Основной игровой цикл
void game_loop() {
    SDL_Event event;
    Uint32 frame_start;
    int frame_time;

    // Сбрасываем флаги перед началом игры
    game_over = false;
    game_won = false;
    return_to_menu = false;
    score = 0;
    init_player();
    init_enemies();
    init_bullets();

    // Игровой цикл
    if (current_state == GAME_MODE_AI_VS_AI) {
        return;
    }



    /* === ИЗМЕРИТЕЛЬ ВРЕМЕНИ КАДРА (для бенчмарка) ===
     * Считаем среднее время одного кадра в миллисекундах.
     * frame_time — это чистое время работы кадра БЕЗ ожидания SDL_Delay,
     * то есть без влияния VSync и частоты обновления монитора.
     * Результат дописывается в файл bench_log.txt, каждую секунду.
     */
    static double bench_sum = 0;
    static int    bench_count = 0;

    while (!game_over && !return_to_menu) {
        frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game_over = true;
                return;
            }
        }

        handle_input();
        if (game_over) break;

        update_bullets();
        update_enemies();
        check_collisions();

        if (check_victory()) {
            game_won = true;
            game_over = true;
        }

        // Отрисовка
        draw_game_frame();

        frame_time = SDL_GetTicks() - frame_start;

        /* === ИЗМЕРИТЕЛЬ ВРЕМЕНИ КАДРА (для бенчмарка) ===
         * Считаем среднее время одного кадра в миллисекундах.
         * frame_time — чистое время работы кадра БЕЗ ожидания SDL_Delay,
         * то есть без влияния VSync и частоты обновления монитора.
         * Результат дописывается в bench_log.txt раз в секунду.
         */
        bench_sum += frame_time;
        bench_count++;
        if (bench_count >= 60) {
            FILE* f = fopen("bench_log.txt", "a");
            if (f) {
                fprintf(f, "avg_frame_ms=%.4f\n", bench_sum / bench_count);
                fclose(f);
            }
            bench_sum = 0;
            bench_count = 0;
        }

        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    // Если вышли в меню по ESC - просто возвращаемся
    if (return_to_menu) {
        current_state = MENU_MAIN;
        return;
    }

    // ЭКРАН ОКОНЧАНИЯ ИГРЫ С ВЫБОРОМ
    int selected_option = 0;  // 0 - Replay, 1 - Menu
    bool choosing = true;
    Uint32 last_key_time = 0;

    while (choosing) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                choosing = false;
                return;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        Uint32 current_time = SDL_GetTicks();

        // Навигация по кнопкам
        if (current_time - last_key_time > 150) {
            if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN] ||
                state[SDL_SCANCODE_W] || state[SDL_SCANCODE_S]) {
                selected_option = 1 - selected_option;
                last_key_time = current_time;
            }
        }

        // Выбор
        if (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_SPACE]) {
            if (selected_option == 0) {
                // REPLAY - перезапускаем игру
                choosing = false;
                game_loop();
                return;
            }
            else {
                // MENU - выходим в главное меню
                choosing = false;
                current_state = MENU_MAIN;
                return;
            }
        }

        // ESC - выход в меню
        if (state[SDL_SCANCODE_ESCAPE]) {
            choosing = false;
            current_state = MENU_MAIN;
            return;
        }

        // Отрисовка экрана результатов
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_stars();

        // Полупрозрачный фон
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &overlay);

        // Рамка
        draw_rect(SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 - 120, 360, 240, COLOR_BLUE);
        draw_rect(SCREEN_WIDTH / 2 - 175, SCREEN_HEIGHT / 2 - 115, 350, 230, COLOR_BLACK);

        // Заголовок
        if (game_won) {
            draw_text("VICTORY!", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 90, COLOR_YELLOW);
        }
        else {
            draw_text("GAME OVER", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 90, COLOR_RED);
        }

        // Счёт
        char score_text[50];
        sprintf(score_text, "SCORE: %d", score);
        draw_text(score_text, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 40, COLOR_WHITE);

        // Кнопка REPLAY
        int button_width = 200;
        int button_height = 40;
        int button_x = SCREEN_WIDTH / 2 - button_width / 2;
        int button_y = SCREEN_HEIGHT / 2 + 10;

        if (selected_option == 0) {
            draw_rect(button_x, button_y, button_width, button_height, COLOR_GREEN);
            draw_text("> REPLAY <", button_x + 55, button_y + 10, COLOR_BLACK);
        }
        else {
            draw_rect(button_x, button_y, button_width, button_height, COLOR_BLUE);
            draw_text("REPLAY", button_x + 70, button_y + 10, COLOR_WHITE);
        }

        // Кнопка MENU
        int button2_y = button_y + 55;

        if (selected_option == 1) {
            draw_rect(button_x, button2_y, button_width, button_height, COLOR_GREEN);
            draw_text("> MENU <", button_x + 65, button2_y + 10, COLOR_BLACK);
        }
        else {
            draw_rect(button_x, button2_y, button_width, button_height, COLOR_BLUE);
            draw_text("MENU", button_x + 80, button2_y + 10, COLOR_WHITE);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
void check_collisions_ai(void) {
    // Только пули игрока уничтожают врагов (без начисления очков)
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && !bullets[b].is_enemy) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (enemies[e].alive) {
                    if (bullets[b].x < enemies[e].x + enemies[e].width &&
                        bullets[b].x + bullets[b].width > enemies[e].x &&
                        bullets[b].y < enemies[e].y + enemies[e].height &&
                        bullets[b].y + bullets[b].height > enemies[e].y) {
                        enemies[e].alive = false;
                        bullets[b].active = false;
                        break;
                    }
                }
            }
        }
    }
    // Игрок неуязвим - проверку попадания в игрока НЕ делаем
}