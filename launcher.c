
#include "brickgame.h"
#include "games/example_game.h"
#include "games/snake.h"
#include "games/tetris.h"

int main() {
  /* добавляем игры */

  /* игра змейка */
  bg_add_game(&snake_init, &snake_destr, &snake_main_loop,
              &snake_on_key_pressed);

  /* игра тетрис */
  bg_add_game(&tg_init, &tg_destr, &tg_game, &tg_on_key_pressed);

  /* пример игры для демонстрации */
  bg_add_game(&example_game_init, &example_game_destr, &example_game_main_loop,
              &example_game_on_key_pressed);

  /* открываем окно */
  bg_init();

  return 0;
}
