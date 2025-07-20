#include "loader.h"

/* индекс выбранной игры */
static int selected_game = 1;

void show_game_number() {
  bg_clear_bricks();
  for (int i = 0; i < NUMBER_SIZE; i++) {
    int x = _numbers_[selected_game][i][0];
    int y = _numbers_[selected_game][i][1];
    if (x < 0 || y < 0) {
      continue;
    }
    bg_bricks[x][y] = CELL_TYPE_FILLED;
  }
}

void ldr_main_loop(unsigned long total_elapsed_ms) {}

void ldr_on_key_pressed(int key_code) {
  if (key_code == KEY_CODE_LEFT || key_code == KEY_CODE_DOWN) {
    selected_game = (selected_game - 1 > 0) ? selected_game -= 1 : GAME_CNT;
    show_game_number();
  } else if (key_code == KEY_CODE_RIGHT || key_code == KEY_CODE_UP) {
    selected_game = (selected_game + 1 < GAME_CNT + 1) ? selected_game += 1 : 1;
    show_game_number();
  } else if (key_code == KEY_CODE_ENTER) {
    /* запуск игры */
    bg_load_game(selected_game);
  }
}

void ldr_init() {
  bg_key_auto_repeat_left = KEY_AUTO_REPEAT_5;
  bg_key_auto_repeat_right = KEY_AUTO_REPEAT_5;
  bg_key_auto_repeat_down = KEY_AUTO_REPEAT_5;
  bg_key_auto_repeat_up = KEY_AUTO_REPEAT_5;

  show_game_number();
}

void ldr_destr() {}