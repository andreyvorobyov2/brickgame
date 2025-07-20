#include "tetris.h"

#define F CELL_TYPE_FILLED
#define E CELL_TYPE_EMPTY
#define X_IDX 0
#define Y_IDX 1
#define T_SIZE 4
#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MOVE_DOWN 2
#define MOVE_ROTATE 3

int t_falling[2][4];  // фигура падающая
int t_tmp[2][4];      // копия падающей фигуры, нужна для проверки столкновений
int t_next[2][4];     // следующая фигура, отобажается в маленьком поле
bool have_next = false;

int angle = 0;  // угол поворота от 0 до 3

int t_falling_idx = 0;  // номер падающего тетромино от 0 до 6
int t_next_idx = 0;     // номер следующего тетромино от 0 до 6

int x_shift = 0;  // смещение падающей фигуры относительно места появления
int y_shift = 0;

unsigned long past_ms = 0;

void set_small_bricks(int t[][4]) {
  for (int i = 0; i < T_SIZE; i++) {
    bg_small_bricks[t[X_IDX][i]][t[Y_IDX][i]] = F;
  }
}

void load_tetromino(int to[][4], int index, int ang, int x_sh, int y_sh) {
  for (int i = 0; i < T_SIZE; i++) {
    to[X_IDX][i] = _tetromino_[index][ang][X_IDX][i] + x_sh;
    to[Y_IDX][i] = _tetromino_[index][ang][Y_IDX][i] + y_sh;
  }
}

/* копирует из t_tmp в t_falling  */
void show_tetramino() {
  for (int i = 0; i < T_SIZE; i++) {
    t_falling[X_IDX][i] = t_tmp[X_IDX][i];
    t_falling[Y_IDX][i] = t_tmp[Y_IDX][i];
    bg_bricks[t_falling[X_IDX][i]][t_falling[Y_IDX][i]] = F;
  }
}

bool have_collisions() {
  for (int i = 0; i < T_SIZE; i++) {
    int x_tmp = t_tmp[X_IDX][i];
    int y_tmp = t_tmp[Y_IDX][i];

    if (x_tmp < 0 || x_tmp >= CELL_X_CNT || y_tmp < 0 || y_tmp >= CELL_Y_CNT) {
      return true;  // столкновение с границами
    }

    bool is_same = false;
    for (int i2 = 0; i2 < T_SIZE; i2++) {
      int x_cur = t_falling[X_IDX][i2];
      int y_cur = t_falling[Y_IDX][i2];
      if (x_tmp == x_cur && y_tmp == y_cur) {
        is_same = true;  // это блоки которые заняты сейчас
        break;
      }
    }
    if (is_same) {
      continue;
    }
    if (bg_bricks[x_tmp][y_tmp] == F) {
      return true;  // столкновения с другими блоками
    }
  }
  return false;  // свободно
}

void new_random_tetromino() {
  if (!have_next) {
    t_next_idx = rand() % (7);
    load_tetromino(t_next, t_next_idx, 0, 0, 0);
    have_next = true;
  }
  angle = 0;
  x_shift = 3;
  y_shift = 0;
  t_falling_idx = t_next_idx;

  load_tetromino(t_tmp, t_falling_idx, angle, x_shift, y_shift);
  if (have_collisions()) {
    bg_game_over = true;
    return;
  }
  show_tetramino();

  t_next_idx = rand() % (7);
  load_tetromino(t_next, t_next_idx, 0, 0, 0);
  bg_clear_small_bricks();
  set_small_bricks(t_next);
}

void clear_line(int line_idx) {
  for (int x = 0; x < CELL_X_CNT; x++) {
    bg_bricks[x][line_idx] = E;
  }
}

void down_upper_lines(int line_idx) {
  for (int y = line_idx; y > 0; y--) {
    for (int x = 0; x < CELL_X_CNT; x++) {
      bg_bricks[x][y] =
          bg_bricks[x][y - 1];  // копируем верхнюю строку в нижнюю
    }
    clear_line(y - 1);  // очищаем вернюю строку
  }
}

void check_lines() {
  int lines_cnt = 0;
  for (int y = 0; y < CELL_Y_CNT; y++) {
    bool all_line_filled = true;
    for (int x = 0; x < CELL_X_CNT; x++) {
      if (bg_bricks[x][y] != F) {
        all_line_filled = false;
        break;
      }
    }
    if (all_line_filled) {
      clear_line(y);
      lines_cnt++;
      down_upper_lines(y);
    }
  }

  bg_level += lines_cnt;

  /* начисление очков */
  switch (lines_cnt) {
    case 1: {
      bg_score += 100;
      break;
    }
    case 2: {
      bg_score += 300;
      break;
    }
    case 3: {
      bg_score += 700;
      break;
    }
    case 4: {
      bg_score += 1500;
      break;
    }
  }
}

void clear_bricks(int t[][4]) {
  for (int i = 0; i < T_SIZE; i++) {
    bg_bricks[t[X_IDX][i]][t[Y_IDX][i]] = E;
  }
}

void do_move(int dir) {
  int angle_tmp = angle;
  if (dir != MOVE_ROTATE) {
    load_tetromino(t_tmp, t_falling_idx, angle, x_shift, y_shift);
    for (int i = 0; i < T_SIZE; i++) {
      switch (dir) {
        case MOVE_LEFT: {
          t_tmp[X_IDX][i] -= 1;
          break;
        }
        case MOVE_RIGHT: {
          t_tmp[X_IDX][i] += 1;
          break;
        }
        case MOVE_DOWN: {
          t_tmp[Y_IDX][i] += 1;
          break;
        }
      }
    }
  } else {  // вращение
    angle_tmp = (angle_tmp + 1 < 4) ? angle_tmp + 1 : 0;

    load_tetromino(t_tmp, t_falling_idx, angle_tmp, x_shift, y_shift);
  }
  if (!have_collisions()) {  // столкновений нет
    clear_bricks(t_falling);
    show_tetramino();

    switch (dir) {
      case MOVE_LEFT: {
        x_shift--;
        break;
      }
      case MOVE_RIGHT: {
        x_shift++;
        break;
      }
      case MOVE_DOWN: {
        y_shift++;
        break;
      }
      case MOVE_ROTATE: {
        angle = angle_tmp;
        break;
      }
    }
  } else {  // есть столкновение
    if (dir == MOVE_DOWN) {
      check_lines();
      bg_reset_last_pressed_key();
      new_random_tetromino();
    }
  }
}

void tg_game(unsigned long total_elapsed_ms) {
  if (total_elapsed_ms - past_ms >= 400) {
    past_ms = total_elapsed_ms;
    do_move(MOVE_DOWN);
  }
}

void tg_on_key_pressed(int key_code) {
  switch (key_code) {
    case KEY_CODE_ENTER: {
      break;
    }
    case KEY_CODE_DOWN: {
      do_move(MOVE_DOWN);
      break;
    }
    case KEY_CODE_UP: {
      do_move(MOVE_ROTATE);
      break;
    }
    case KEY_CODE_LEFT: {
      do_move(MOVE_LEFT);
      break;
    }
    case KEY_CODE_RIGHT: {
      do_move(MOVE_RIGHT);
      break;
    }
  }
}

void tg_init() {
  bg_key_auto_repeat_left = KEY_AUTO_REPEAT_4;
  bg_key_auto_repeat_right = KEY_AUTO_REPEAT_4;
  bg_key_auto_repeat_down = KEY_AUTO_REPEAT_1;

  have_next = false;
  past_ms = 0;
  new_random_tetromino();
}

void tg_destr() {}