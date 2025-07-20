#ifndef __SNAKE__
#define __SNAKE__

#include "../brickgame.h"

/* эта функция будет вызывана один раз при начале игры
 */
extern void snake_init();

/* эта функция будет вызывана в конце игры
 * в ней можно очистить динамическую память
 */
extern void snake_destr();

/* эта функция будет вызыватся каждые 40 мс.
 *  elapsed_ms : кол-во миллисекунд прощедщих с начала запуска игры
 */
extern void snake_main_loop(unsigned long elapsed_ms);

/* эта функция будет взывана при нажатии клавиши
 *  key_code : код нажатой клавиши, одно из значений
 *  KEY_CODE_UP, KEY_CODE_DOWN, KEY_CODE_LEFT, KEY_CODE_RIGHT, KEY_CODE_ENTER
 */
extern void snake_on_key_pressed(int key_code);

#endif