
#include "snake.h"

/* направления движения */
#define MOVE_UP 0
#define MOVE_DOWN 1
#define MOVE_LEFT 2
#define MOVE_RIGHT 3

/* максимальный размер змеи */
#define SNAKE_MAX_SIZE 100

#define _X 0
#define _Y 1

/* координаты клетки */
typedef struct {
  int x;
  int y;
} Point;

static int size;                    // размер змеи, сколько клеток вдлину
static Point body[SNAKE_MAX_SIZE];  // массив с коодинатами тела змеи
static int dir;                     // направление движения
static int new_dir;  // новое направление движения, считанное с клавиатуры
Point eat;           // еда которую ест змея

/* генерирует случайные координаты для еды */
Point put_eat() {
  // Задаем область допустимых координат
  int min_x = 0, max_x = CELL_X_CNT - 1;
  int min_y = 0, max_y = CELL_Y_CNT - 1;

  // Задаем массив занятых координат
  int occupied_coordinates[size][2];
  for (int i = 0; i < size; i++) {
    occupied_coordinates[i][_X] = body[i].x;  // x
    occupied_coordinates[i][_Y] = body[i].y;  // y
  }
  int num_occupied =
      sizeof(occupied_coordinates) / sizeof(occupied_coordinates[0]);

  // Генерируем случайные координаты
  int random_x, random_y;
  bg_generate_random_coordinate(&random_x, &random_y, min_x, max_x, min_y,
                                max_y, occupied_coordinates, num_occupied);

  Point eat;
  eat.x = random_x;
  eat.y = random_y;
  return eat;
}

/* эта функция будет вызывана один раз при начале игры
 */
void snake_init() {
  size = 3;  // при старте змея состоит из трех клеток

  /* первоначальные координаты змеи */
  body[0].x = 4;  // голова
  body[0].y = 17;

  body[1].x = 4;
  body[1].y = 18;

  body[2].x = 4;  // хвост
  body[2].y = 19;

  /* сбрасываем показатели игры */
  bg_score = 0;
  bg_level = 0;
  bg_speed = 1;  // начинать будем с первой скорости

  /* змея ориентирована головой вверх, начальное движение будет вверх */
  dir = MOVE_UP;
  new_dir = MOVE_UP;

  /* показываем змею на игровом поле */
  for (int i = 0; i < size; i++) {
    bg_bricks[body[i].x][body[i].y] = CELL_TYPE_FILLED;
  }

  /* помещаем еду в случайные координаты */
  eat = put_eat();

  /* показваем еду на игровом поле, она будет мигать */
  bg_bricks[eat.x][eat.y] = CELL_TYPE_BLINK;
}

/* эта функция будет вызывана в конце игры
 * в ней можно очистить динамическую память
 */
void snake_destr() {}

/* эта функция будет вызыватся каждые 40 мс.
 *  elapsed_ms : кол-во миллисекунд прощедщих с начала запуска игры
 */
void snake_main_loop(unsigned long elapsed_ms) {
  static unsigned long past_ms = 0;

  if (elapsed_ms - past_ms > 200) {
    past_ms = elapsed_ms;
  } else {
    return;  // время двигать змею еще не пришло
  }

  /* координаты головы */
  Point head = body[0];

  /* координаты хвоста */
  Point tail0 = body[size - 1];
  Point tail1 = body[size - 2];

  /* новые координаты головы */
  bool is_reverse = false;
  switch (new_dir) {
    case MOVE_UP: {
      if (dir == MOVE_DOWN) {
        is_reverse = true;
      } else {
        head.y -= 1;
      }
      break;
    }
    case MOVE_DOWN: {
      if (dir == MOVE_UP) {
        is_reverse = true;
      } else {
        head.y += 1;
      }
      break;
    }
    case MOVE_LEFT: {
      if (dir == MOVE_RIGHT) {
        is_reverse = true;
      } else {
        head.x -= 1;
      }
      break;
    }
    case MOVE_RIGHT: {
      if (dir == MOVE_LEFT) {
        is_reverse = true;
      } else {
        head.x += 1;
      }
      break;
    }
  }

  /* включаем задний ход */
  if (is_reverse) {
    /* куда движется хвост */
    if (tail0.y > tail1.y) {  //  вверх
      new_dir = MOVE_DOWN;
    } else if (tail0.y < tail1.y) {  // вниз
      new_dir = MOVE_UP;
    } else if (tail0.x > tail1.x) {  // влево
      new_dir = MOVE_RIGHT;
    } else if (tail0.x < tail1.x) {  // вправо
      new_dir = MOVE_LEFT;
    }

    /* реверсируем массив, через временный массив */
    Point tmp_body[size];
    for (int i = 0; i < size; i++) {
      tmp_body[i] = body[size - 1 - i];
    }

    for (int i = 0; i < size; i++) {
      body[i] = tmp_body[i];
    }

    /* координаты головы */
    head = body[0];

    /* координаты хвоста */
    tail0 = body[size - 1];

    switch (new_dir) {
      case MOVE_UP: {
        head.y -= 1;
        break;
      }
      case MOVE_DOWN: {
        head.y += 1;
        break;
      }
      case MOVE_LEFT: {
        head.x -= 1;
        break;
      }
      case MOVE_RIGHT: {
        head.x += 1;
        break;
      }
    }
  }

  dir = new_dir;

  /* врезались в стенки игового поля */
  if (head.x < 0 || head.x >= CELL_X_CNT || head.y < 0 ||
      head.y >= CELL_Y_CNT) {
    bg_game_over = true;  // ну вот и все
    return;
  }

  /* врезались в собственное тело */
  for (int i = 0; i < size; i++) {
    if (head.x == body[i].x && head.y == body[i].y) {
      bg_game_over = true;
      return;
    }
  }

  /* заполним клетку с новыми координатами головы */
  bg_bricks[head.x][head.y] = CELL_TYPE_FILLED;

  /* координаты всех клеток меняются на координаты соседней клетки которая ближе
   * к голове */
  for (int i = 0; i < size; i++) {
    Point tmp = body[i];
    body[i] = head;
    head = tmp;
  }

  /* голова на еде, съели еду */
  if (body[0].x == eat.x && body[0].y == eat.y) {
    bg_level++;                       // увеличем уровень
    bg_score += bg_speed * bg_level;  // увеличем очки

    /* новая еда */
    eat = put_eat();
    bg_bricks[eat.x][eat.y] = CELL_TYPE_BLINK;

    /* последняя клетка в хвосте не удаляется, так растет змея */
    size++;
    body[size - 1] = tail0;

  } else {
    /* не врезались, но и еду не съели, очистим клетку с хвостом */
    if (!(tail0.x == eat.x && tail0.y == eat.y)) {
      bg_bricks[tail0.x][tail0.y] = CELL_TYPE_EMPTY;
    }
  }
}

/* эта функция будет взывана при нажатии клавиши
 *  key_code : код нажатой клавиши, одно из значений
 *  KEY_CODE_UP, KEY_CODE_DOWN, KEY_CODE_LEFT, KEY_CODE_RIGHT, KEY_CODE_ENTER
 */
void snake_on_key_pressed(int key_code) {
  switch (key_code) {
    case KEY_CODE_UP: {
      new_dir = MOVE_UP;
      break;
    }
    case KEY_CODE_DOWN: {
      new_dir = MOVE_DOWN;
      break;
    }
    case KEY_CODE_LEFT: {
      new_dir = MOVE_LEFT;
      break;
    }
    case KEY_CODE_RIGHT: {
      new_dir = MOVE_RIGHT;
      break;
    }
  }
}

#undef MOVE_UP
#undef MOVE_DOWN
#undef MOVE_LEFT
#undef MOVE_RIGHT
#undef _X
#undef _Y
