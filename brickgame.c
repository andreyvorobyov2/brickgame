#include "brickgame.h"

Display* display;
int screen;
Window window;
GC gc;
XkbDescPtr xkb;

/* шрифты */
XFontStruct* font_info;

/* цвета */
unsigned long gray_color, black_color, background_color;

/* указатель на функцию с инициализацией новой игры */
void (*game_init_ptr[GAME_CNT + 1])();
/* указатель на функцию с деструктором игры */
void (*game_destructor_ptr[GAME_CNT + 1])();
/* указатель на функцию с логикой игры */
void (*game_main_loop_ptr[GAME_CNT + 1])(unsigned long total_elapsed_ms);
/* указатель на функцию когда нажата клавиша */
void (*game_on_key_pressed_ptr[GAME_CNT + 1])(int key_code);

bool blink = false;  // виден или нет мигающий блок

int bg_bricks[CELL_X_CNT][CELL_Y_CNT];
int cache_bricks[CELL_X_CNT][CELL_Y_CNT];

int bg_small_bricks[SMALL_CELL_X_CNT][SMALL_CELL_Y_CNT];
int cache_small_bricks[SMALL_CELL_X_CNT][SMALL_CELL_Y_CNT];

/* текущая игра */
int game_index = 0;

bool bg_game_over = false;
bool bg_pause = false;

int bg_score = 0;
int bg_level = 0;
int bg_speed = 0;

/* для отрисовки игового поля */
int rec_f_idx = 0;
XRectangle rec_f[CELL_CNT];

int seg_f_idx = 0;
XSegment seg_f[(CELL_CNT * 4)];

int rec_e_idx = 0;
XRectangle rec_e[CELL_CNT];

int seg_e_idx = 0;
XSegment seg_e[CELL_CNT * 4];

/* для отисовки поля следующей фигуры */
int small_rec_f_idx = 0;
XRectangle small_rec_f[SMALL_CELL_CNT];

int small_seg_f_idx = 0;
XSegment small_seg_f[(SMALL_CELL_CNT * 4)];

int small_rec_e_idx = 0;
XRectangle small_rec_e[SMALL_CELL_CNT];

int small_seg_e_idx = 0;
XSegment small_seg_e[SMALL_CELL_CNT * 4];

int key_code = KEY_CODE_NONE;
bool key_pressed = false;

int last_presed_key_code = KEY_CODE_NONE;

int bg_key_auto_repeat_left = KEY_AUTO_REPEAT_NONE;
int bg_key_auto_repeat_right = KEY_AUTO_REPEAT_NONE;
int bg_key_auto_repeat_up = KEY_AUTO_REPEAT_NONE;
int bg_key_auto_repeat_down = KEY_AUTO_REPEAT_NONE;

void create_colors() {
  Colormap clrmap = DefaultColormap(display, screen);

  XColor color, exact;

  char background[] = "#9EAD86";
  XParseColor(display, clrmap, background, &color);
  XAllocColor(display, clrmap, &color);
  background_color = color.pixel;

  char gray[] = "#919E78";
  XParseColor(display, clrmap, gray, &color);
  XAllocColor(display, clrmap, &color);
  gray_color = color.pixel;

  XAllocNamedColor(display, clrmap, "Black", &color, &exact);
  black_color = color.pixel;
}

void create_window() {
  // создаем окно, изменять размеры нельзя
  window = XCreateSimpleWindow(
      display, RootWindow(display, screen), 0,
      0,  // позиция на дисплее, установка позиции окна так не работает, после
          // открытия перемещаем окно XMoveWindow
      WINDOW_WIDTH, WINDOW_HEIGHT,  // ширина и высота окна
      0,                            // ширина рамки окна
      BlackPixel(display, screen),  // цвет рамки окна
      background_color);            // цвет фона окна

  // свойства размера окна
  XSizeHints* size_hints = XAllocSizeHints();
  // изменять размер окна нельзя
  size_hints->flags = PMinSize | PMaxSize;  // нужно указать какие параметры
                                            // задаются, мин. макс. размер окна
  size_hints->min_height = size_hints->max_height = WINDOW_HEIGHT;
  size_hints->min_width = size_hints->max_width = WINDOW_WIDTH;
  XSetWMNormalHints(display, window, size_hints);
}

// проверка, занята ли координата
bool is_coordinate_occupied(int x,
                            int y,
                            int occupied_coordinates[][2],
                            int num_occupied) {
  for (int i = 0; i < num_occupied; i++) {
    if (occupied_coordinates[i][0] == x && occupied_coordinates[i][1] == y) {
      return true;  // занято
    }
  }
  return false;  // свободно
}

// генерация случайных координат
void bg_generate_random_coordinate(int* x,
                                   int* y,
                                   int min_x,
                                   int max_x,
                                   int min_y,
                                   int max_y,
                                   int occupied_coordinates[][2],
                                   int num_occupied) {
  do {
    *x = min_x + rand() % (max_x - min_x + 1);
    *y = min_y + rand() % (max_y - min_y + 1);
  } while (is_coordinate_occupied(*x, *y, occupied_coordinates, num_occupied));
}

/* объединяет строку и число в строку */
char* concat_str_int(char* str_val, int int_val) {
  char* str1 = strdup(str_val);
  char* str2 = NULL;

  // Преобразуем число в строку
  int len = snprintf(NULL, 0, "%d", int_val);  // Узнаем длину строки
  str2 = (char*)malloc(len + 1);               // Выделяем память
  snprintf(str2, len + 1, "%d", int_val);      // Записываем строку

  // Объединяем строки
  char* result = (char*)malloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  // Освобождаем выделенную память
  free(str1);
  free(str2);

  return result;
}

void bg_clear_bricks() {
  for (int x = 0; x < CELL_X_CNT; x++) {
    for (int y = 0; y < CELL_Y_CNT; y++) {
      bg_bricks[x][y] = CELL_TYPE_NONE;
    }
  }
}

void bg_clear_small_bricks() {
  for (int x = 0; x < SMALL_CELL_X_CNT; x++) {
    for (int y = 0; y < SMALL_CELL_Y_CNT; y++) {
      bg_small_bricks[x][y] = CELL_TYPE_NONE;
    }
  }
}

/* верхний левый, нижний левый */
void create_segment_0(XRectangle r, XSegment* seg, int* seg_idx, int padding) {
  XSegment s0;
  s0.x1 = r.x - padding;
  s0.y1 = r.y - padding;
  s0.x2 = r.x - padding;
  s0.y2 = r.y + r.height + padding;
  seg[*seg_idx] = s0;
  *seg_idx = *seg_idx + 1;
}

/* верхний левый, верхний правый */
void create_segment_1(XRectangle r, XSegment* seg, int* seg_idx, int padding) {
  XSegment s1;
  s1.x1 = r.x - padding;
  s1.y1 = r.y - padding;
  s1.x2 = r.width + r.x + padding;
  s1.y2 = r.y - padding;
  seg[*seg_idx] = s1;
  *seg_idx = *seg_idx + 1;
}

/* верхний правый, нижний правый */
void create_segment_2(XRectangle r, XSegment* seg, int* seg_idx, int padding) {
  XSegment s2;
  s2.x1 = r.width + r.x + padding;
  s2.y1 = r.y - padding;
  s2.x2 = r.width + r.x + padding;
  s2.y2 = r.height + r.y + padding;
  seg[*seg_idx] = s2;
  *seg_idx = *seg_idx + 1;
}

/* нижний левый, нижний правый */
void create_segment_3(XRectangle r, XSegment* seg, int* seg_idx, int padding) {
  XSegment s3;
  s3.x1 = r.x - padding;
  s3.y1 = r.y + r.height + padding;
  s3.x2 = r.width + r.x + padding;
  s3.y2 = r.height + r.y + padding;
  seg[*seg_idx] = s3;
  *seg_idx = *seg_idx + 1;
}

void create_brick(int x,
                  int y,
                  XRectangle* rec,
                  int* rec_idx,
                  XSegment* seg,
                  int* seg_idx,
                  int cell_type) {
  if (cache_bricks[x][y] == cell_type) {
    return;
  }
  cache_bricks[x][y] = cell_type;

  XRectangle r;
  r.x = 5 + (x * CELL_SIZE) + GRID_LEFT;
  r.y = 5 + (y * CELL_SIZE) + GRID_TOP;
  r.width = CELL_SIZE - 10;
  r.height = CELL_SIZE - 10;
  rec[*rec_idx] = r;
  *rec_idx = *rec_idx + 1;

  const int padding = 3;

  create_segment_0(r, seg, seg_idx, padding);
  create_segment_1(r, seg, seg_idx, padding);
  create_segment_2(r, seg, seg_idx, padding);
  create_segment_3(r, seg, seg_idx, padding);
}

void create_small_brick(int x,
                        int y,
                        XRectangle* rec,
                        int* rec_idx,
                        XSegment* seg,
                        int* seg_idx,
                        int cell_type) {
  if (cache_small_bricks[x][y] == cell_type) {
    return;
  }
  cache_small_bricks[x][y] = cell_type;

  XRectangle r;
  r.x = 5 + (x * SMALL_CELL_SIZE) + SMALL_GRID_LEFT;
  r.y = 5 + (y * SMALL_CELL_SIZE) + SMALL_GRID_TOP;
  r.width = SMALL_CELL_SIZE - 10;
  r.height = SMALL_CELL_SIZE - 10;
  rec[*rec_idx] = r;
  *rec_idx = *rec_idx + 1;

  const int padding = 3;

  create_segment_0(r, seg, seg_idx, padding);
  create_segment_1(r, seg, seg_idx, padding);
  create_segment_2(r, seg, seg_idx, padding);
  create_segment_3(r, seg, seg_idx, padding);
}

/* рисует правую панель */
void draw_right_panel() {
  XSetForeground(display, gc, black_color);

  /* высота строки в пикселях */
  const int font_height = font_info->ascent + font_info->descent;

  /* координаты написи xy */
  const int str_x = GRID_LEFT + (CELL_X_CNT * CELL_SIZE) + 10;
  const int str_y = GRID_TOP;

  XClearArea(display, window, str_x, str_y, 200, 70, false);

  /* первая строка Score */
  char* score_str = concat_str_int("SCORE: ", bg_score);
  XDrawString(display, window, gc, str_x, str_y + font_height, score_str,
              strlen(score_str));

  /* вторая строка Level */
  char* level_str = concat_str_int("LEVEL: ", bg_level);
  XDrawString(display, window, gc, str_x, str_y + (font_height * 2), level_str,
              strlen(level_str));

  /* тетья строка Speed */
  char* speed_str = concat_str_int("SPEED: ", bg_speed);
  XDrawString(display, window, gc, str_x, str_y + (font_height * 3), speed_str,
              strlen(speed_str));

  /* стоки GAME OVER и PAUSE выводятся в одном месте, вместе они быть не могут
   */

  /*  строка GAME OVER */
  if (bg_game_over && blink) {
    char* game_over_str = "GAME OVER";
    XDrawString(display, window, gc, str_x, str_y + (font_height * 4),
                game_over_str, strlen(game_over_str));
  }

  /*  строка PAUSE */
  if (bg_pause && blink) {
    char* pause_str = "PAUSE";
    XDrawString(display, window, gc, str_x, str_y + (font_height * 4),
                pause_str, strlen(pause_str));
  }

  // return;

  /* маленькое поле, показывающее следующую фигуру */
  small_rec_f_idx = 0;
  small_seg_f_idx = 0;
  small_rec_e_idx = 0;
  small_seg_e_idx = 0;

  for (int x = 0; x < SMALL_CELL_X_CNT; x++) {
    for (int y = 0; y < SMALL_CELL_Y_CNT; y++) {
      if (bg_small_bricks[x][y] == CELL_TYPE_FILLED ||
          (blink && bg_small_bricks[x][y] == CELL_TYPE_BLINK)) {
        create_small_brick(x, y, small_rec_f, &small_rec_f_idx, small_seg_f,
                           &small_seg_f_idx, CELL_TYPE_FILLED);
      } else {
        create_small_brick(x, y, small_rec_e, &small_rec_e_idx, small_seg_e,
                           &small_seg_e_idx, CELL_TYPE_EMPTY);
      }
    }
  }

  if (small_rec_f_idx != 0) {
    XFillRectangles(display, window, gc, small_rec_f, small_rec_f_idx);
  }
  if (small_seg_f_idx != 0) {
    XDrawSegments(display, window, gc, small_seg_f, small_seg_f_idx);
  }

  /* серым цветом все невидимые блоки */
  XSetForeground(display, gc, gray_color);

  if (small_rec_e_idx != 0) {
    XFillRectangles(display, window, gc, small_rec_e, small_rec_e_idx);
  }
  if (small_seg_e_idx != 0) {
    XDrawSegments(display, window, gc, small_seg_e, small_seg_e_idx);
  }
}

void draw() {
  rec_f_idx = 0;
  seg_f_idx = 0;
  rec_e_idx = 0;
  seg_e_idx = 0;

  for (int x = 0; x < CELL_X_CNT; x++) {
    for (int y = 0; y < CELL_Y_CNT; y++) {
      if (bg_bricks[x][y] == CELL_TYPE_FILLED ||
          (blink && bg_bricks[x][y] == CELL_TYPE_BLINK)) {
        create_brick(x, y, rec_f, &rec_f_idx, seg_f, &seg_f_idx,
                     CELL_TYPE_FILLED);
      } else {
        create_brick(x, y, rec_e, &rec_e_idx, seg_e, &seg_e_idx,
                     CELL_TYPE_EMPTY);
      }
    }
  }

  draw_right_panel();

  /* черным цветом все видимые блоки */
  XSetForeground(display, gc, black_color);

  if (rec_f_idx != 0) {
    XFillRectangles(display, window, gc, rec_f, rec_f_idx);
  }
  if (seg_f_idx != 0) {
    XDrawSegments(display, window, gc, seg_f, seg_f_idx);
  }

  /* серым цветом все невидимые блоки */
  XSetForeground(display, gc, gray_color);

  if (rec_e_idx != 0) {
    XFillRectangles(display, window, gc, rec_e, rec_e_idx);
  }
  if (seg_e_idx != 0) {
    XDrawSegments(display, window, gc, seg_e, seg_e_idx);
  }

  /* обновляем дисплей */
  XFlush(display);
}

void bg_reset_last_pressed_key() {
  last_presed_key_code = KEY_CODE_NONE;
}

// количество милли сек. прощедших с начала игры
unsigned long total_elapsed_ms = 0;
unsigned long blink_elapsed_ms = 0;
unsigned long key_auto_repeat_ms = 0;

void* main_thread() {
  srand(time(NULL));
  usleep(100);

  // толщина линий 1 пикселя
  XSetLineAttributes(display, gc, 1, LineSolid, CapRound, JoinRound);

  /* боковые 4 грани */
  XSegment b[4];
  b[0].x1 = X0;
  b[0].y1 = Y0;
  b[0].x2 = X0;
  b[0].y2 = Y1;
  b[1].x1 = X1;
  b[1].y1 = Y0;
  b[1].x2 = X1;
  b[1].y2 = Y1;
  b[2].x1 = X0;
  b[2].y1 = Y0;
  b[2].x2 = X1;
  b[2].y2 = Y0;
  b[3].x1 = X0;
  b[3].y1 = Y1;
  b[3].x2 = X1;
  b[3].y2 = Y1;
  XDrawSegments(display, window, gc, &b[0], 4);

  // толщина линий 2 пикселя
  XSetLineAttributes(display, gc, 2, LineSolid, CapRound, JoinRound);

  // количество милли сек. прощедших с начала игры
  total_elapsed_ms = 0;
  blink_elapsed_ms = 0;
  key_auto_repeat_ms = 0;

  game_init_ptr[game_index]();  // инициализируем игру

  while (true) {  // цикл игры
    /* обновление графики 25 раз в секнду, каждые 40 мс */
    usleep(FPS * 1000);

    blink_elapsed_ms += FPS;
    /* скорость мигания */
    if (blink_elapsed_ms >= 120) {
      blink = !blink;
      blink_elapsed_ms = 0;
    }

    /* автоповтор нажатия клавиш */
    if (last_presed_key_code) {
      unsigned long ar_ms = 0;
      switch (last_presed_key_code) {
        case KEY_CODE_LEFT: {
          ar_ms = bg_key_auto_repeat_left;
          break;
        }
        case KEY_CODE_RIGHT: {
          ar_ms = bg_key_auto_repeat_right;
          break;
        }
        case KEY_CODE_UP: {
          ar_ms = bg_key_auto_repeat_up;
          break;
        }
        case KEY_CODE_DOWN: {
          ar_ms = bg_key_auto_repeat_down;
          break;
        }
      }

      if (ar_ms != 0) {
        key_auto_repeat_ms += FPS;
        /* скорость автоповторения нажатия клавиши */
        if (key_auto_repeat_ms >= ar_ms) {
          key_code = last_presed_key_code;
          key_auto_repeat_ms = 0;
        }
      }
    }

    /* какая-то клавиша нажата */
    if (key_code != KEY_CODE_NONE) {
      if (key_code == KEY_CODE_ENTER) {
        /* запуск загрузчика после game over */
        if (bg_game_over) {
          game_destructor_ptr[game_index]();

          total_elapsed_ms = 0;
          blink_elapsed_ms = 0;
          bg_load_game(0);

          /* игра идет, нажата клавиша Enter - это пауза  */
        } else if (game_index != 0) {
          bg_pause = !bg_pause;
        } else {
          /* запуск новой игры */
          game_on_key_pressed_ptr[game_index](key_code);
        }
        /* нажата клавиша стрелки */
      } else {
        if (!bg_game_over) {
          key_auto_repeat_ms = 0;
          last_presed_key_code = key_code;
          game_on_key_pressed_ptr[game_index](key_code);
        }
      }

      key_code = KEY_CODE_NONE;
    }

    /* игра не окончена и пауза не нажата */
    if (!bg_game_over && !bg_pause) {
      total_elapsed_ms += FPS;  // прошло 40 мс.
      game_main_loop_ptr[game_index](total_elapsed_ms);
    }

    draw();
  }
}

void bg_add_game(void (*init_prt)(),
                 void (*destr_pt)(),
                 void (*main_loop_ptr)(unsigned long),
                 void (*on_key_pressed_ptr)(int)) {
  static int loader_index = 0;

  if (loader_index == 0) {
    game_init_ptr[loader_index] = ldr_init;
    game_destructor_ptr[loader_index] = ldr_destr;
    game_main_loop_ptr[loader_index] = ldr_main_loop;
    game_on_key_pressed_ptr[loader_index] = ldr_on_key_pressed;
    loader_index++;
  }
  game_init_ptr[loader_index] = init_prt;
  game_destructor_ptr[loader_index] = destr_pt;
  game_main_loop_ptr[loader_index] = main_loop_ptr;
  game_on_key_pressed_ptr[loader_index] = on_key_pressed_ptr;
  loader_index++;
}

void bg_load_game(int index) {
  game_index = index;

  bg_level = 0;
  bg_score = 0;
  bg_speed = 0;

  bg_game_over = false;
  bg_pause = false;

  bg_clear_bricks();
  bg_clear_small_bricks();

  game_init_ptr[game_index]();

  total_elapsed_ms = 0;
  blink_elapsed_ms = 0;
  key_auto_repeat_ms = 0;
}
// int counter=0;

void bg_init() {
  /* инициализируем массивы */
  for (int x = 0; x < CELL_X_CNT; x++) {
    for (int y = 0; y < CELL_Y_CNT; y++) {
      bg_bricks[x][y] = CELL_TYPE_NONE;
      cache_bricks[x][y] = CELL_TYPE_NONE;
    }
  }

  for (int x = 0; x < SMALL_CELL_X_CNT; x++) {
    for (int y = 0; y < SMALL_CELL_Y_CNT; y++) {
      bg_small_bricks[x][y] = CELL_TYPE_NONE;
      cache_small_bricks[x][y] = CELL_TYPE_NONE;
    }
  }

  /* включаем моготочность */
  XInitThreads();

  /* открываем соединение с X-сервером */
  display = XOpenDisplay(NULL);
  Bool supported = false;
  XkbSetDetectableAutoRepeat(display, true, &supported);

  /* создаем цвета
     переменные: gray_color, black_color, background_color*/
  create_colors();

  /* получаем номер экрана по умолчанию */
  screen = DefaultScreen(display);

  /* создаем окно
     переменная: Window*/
  create_window();

  /* создаем графический контекст */
  unsigned long valuemask = 0;
  XGCValues values;
  gc = XCreateGC(display, window, valuemask, &values);

  /* создаем и устанавливаем шрифт коорым будут выводится надписи
   спсок всех доступных шрифтов можно поличить командой xlsfonts в консоли */
  char* font_name = "9x15bold";
  font_info = XLoadQueryFont(display, font_name);
  XSetFont(display, gc, font_info->fid);

  /* выбираем события, которые мы хотим обрабатывать (нажатие клавиши) */
  XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

  /* показываем окно */
  XMapWindow(display, window);

  /* Окно откроется в позиции которую назначит оконный менеджер
       переместим открытое окно в центр дисплея */
  Screen* _screen = XScreenOfDisplay(display, screen);
  // центр дисплея минус половина длины и ширины окна
  int x_pos = (_screen->width / 2) - (WINDOW_WIDTH / 2);
  int y_pos = (_screen->height / 2) - (WINDOW_HEIGHT / 2);
  /* перемещаем окно в центр дисплея */
  XMoveWindow(display, window, x_pos, y_pos);

  /* в отдельном потоке запускаем логику игры */
  pthread_t thread;
  pthread_create(&thread, NULL, main_thread, NULL);
  pthread_detach(thread);

  XEvent event;

  /* цикл обработки событий */
  while (1) {
    XNextEvent(display, &event);

    if (event.type == KeyPress && !key_pressed) {
      key_pressed = true;
      key_code = (int)event.xkey.keycode;
      // printf("%s %d %d\n", "K:", counter, key_code);
      // counter++;
    }

    if (event.type == KeyRelease) {
      key_pressed = false;
      last_presed_key_code = KEY_CODE_NONE;
      // printf("%s %d\n", "RELEASED:", counter);
      // counter++;
    }
  }
}