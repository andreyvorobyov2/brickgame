#ifndef __BRICKGAME__
#define __BRICKGAME__

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h> /* для запуска парралельных потоков */
#include <stdbool.h> /* для типа bool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* дя аботы со строками */
#include <time.h>   /* для замера времени в милли сек. */
#include <unistd.h> /* для функции usleep() */
#include "loader.h"

#define WINDOW_WIDTH 350   // ширина окна в пикселях
#define WINDOW_HEIGHT 420  // высота окна в пикселях

/* позиция верхнего левого угла сетки, пиксели */
#define GRID_TOP 10
#define GRID_LEFT 10

/* большое игровое поле */
#define CELL_X_CNT 10 /* количество ячеек по горизонтали */
#define CELL_Y_CNT 20 /* количество ячеек по вертикали */
#define CELL_SIZE 20  /* размер одной квадратной ячейки в пикселях */
#define CELL_CNT CELL_X_CNT* CELL_Y_CNT

/* маленькое поле со следующей фигурой */
#define SMALL_GRID_TOP 80
#define SMALL_GRID_LEFT 220
#define SMALL_CELL_X_CNT 4
#define SMALL_CELL_Y_CNT 4
#define SMALL_CELL_SIZE 15  // размер в пикселях
#define SMALL_CELL_CNT SMALL_CELL_X_CNT* SMALL_CELL_Y_CNT

/* координаты углов игового поля */
#define X0 GRID_LEFT
#define X1 GRID_LEFT + (CELL_X_CNT * CELL_SIZE)
#define Y0 GRID_TOP
#define Y1 GRID_TOP + (CELL_Y_CNT * CELL_SIZE)

/* тип ячейки, пустая, закрашенная, мигающая */
#define CELL_TYPE_EMPTY 0
#define CELL_TYPE_FILLED 1
#define CELL_TYPE_BLINK 2
#define CELL_TYPE_NONE -1  // значение инициализации

/* количество игр */
#define GAME_CNT 3

/* задержка в мс. пеед вызовом функции с логикой игры */
#define FPS 40

/* коды клавиш клавиатуры */
#define KEY_CODE_UP 111
#define KEY_CODE_DOWN 116
#define KEY_CODE_LEFT 113
#define KEY_CODE_RIGHT 114
#define KEY_CODE_ENTER 36 /* клавиша Enter старт-пауза */
#define KEY_CODE_NONE 0   /* никакая клавиша */

/* скорость авто повторения нажатия клавиши */
#define KEY_AUTO_REPEAT_NONE 0;  // нет автоповтора
#define KEY_AUTO_REPEAT_1 FPS;   // самая быстая скорость
#define KEY_AUTO_REPEAT_2 FPS * 2;
#define KEY_AUTO_REPEAT_3 FPS * 3;
#define KEY_AUTO_REPEAT_4 FPS * 4;
#define KEY_AUTO_REPEAT_5 FPS * 20;  // самая медленная скорость

extern int bg_bricks[CELL_X_CNT][CELL_Y_CNT];
extern int bg_small_bricks[SMALL_CELL_X_CNT][SMALL_CELL_Y_CNT];

/* если записать в переменную true то игра будет остановлена и выведена надпись
 * game over */
extern bool bg_game_over;

/* показатели выводимые на экран как надписи */
extern int bg_score;
extern int bg_level;
extern int bg_speed;

/* включение и скорость автоповтора для клавиш */
extern int bg_key_auto_repeat_left;
extern int bg_key_auto_repeat_right;
extern int bg_key_auto_repeat_up;
extern int bg_key_auto_repeat_down;

/* сбрасывает последнюю нажатую кнопку, останавливает автоповтор */
void bg_reset_last_pressed_key();

void bg_add_game(
    // функция с инициаизацией игры
    void (*init_prt)(),
    // функция с дестуктором игры
    void (*destr_ptr)(),
    // функция с логикой игры
    void (*main_loop_ptr)(unsigned long total_elapsed_ms),
    // вызывается при нажатии клавиши
    void (*on_key_pressed_ptr)(int key_code));

void bg_init();

/* загружает игру */
void bg_load_game(int index);

/* генеирует случайную координату и помещает ее в параметры x,y */
void bg_generate_random_coordinate(int* x,
                                   int* y,
                                   int min_x,
                                   int max_x,
                                   int min_y,
                                   int max_y,
                                   int occupied_coordinates[][2],
                                   int num_occupied);

/* полностью очищает все поле */
void bg_clear_bricks();
void bg_clear_small_bricks();

#endif