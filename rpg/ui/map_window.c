#include "map_window.h"
#include "logger.h"
#include "map.h"
#include "ui_point.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct MapWindow {
  WINDOW  *_window;
  WINDOW  *_inner_window;
  Map     *_map; // This map is owned by an engine, we *must not* free it!
  UiPoint *_ui_point;
  int      _lines;
  int      _cols;
};

bool map_window_check_win_size(Map *map, int lines, int cols) {
  MapBoundaries map_boundaries = map_get_boundaries(map);
  bool          lines_ok = lines > map_boundaries.y + 2;
  bool          cols_ok = cols > map_boundaries.x + 2;

  return lines_ok && cols_ok;
}

MapWindow *map_window_new(Map *map, int lines, int cols, int x, int y) {
  MapWindow    *ret = calloc(1, sizeof(MapWindow));
  MapBoundaries boundaries = map_get_boundaries(map);
  LOG_INFO("Creating map window (%dx%d)", lines, cols);

  if (!map_window_check_win_size(map, lines, cols)) {
    LOG_CRITICAL("Winsize (%dx%d) is too small to draw map and border", cols, lines);
    free(ret);
    return nullptr;
  }

  ret->_ui_point = ui_point_new(x, y);
  ret->_cols = cols;
  ret->_lines = lines;

  // Before you ask: no, it is not a bug. NCurses has `y` and `x` reversed.
  ret->_window = newwin(ret->_lines, ret->_cols, y, x);
  ret->_window->_parent = stdscr;
  ret->_inner_window = subwin(ret->_window, boundaries.y + 2, boundaries.x + 2, y + 1, x + 1);
  ret->_map = map;

  return ret;
}

inline WINDOW *map_window_get_ncurses_window(MapWindow *map_window) {
  return map_window->_window;
}

void map_window_draw_borders(MapWindow *map_window) {
  // clang-format off
  LOG_DEBUG("Drawing borders: %d, %d, %d, %d",
            map_window->_window->_begx,
            map_window->_window->_begy,
            map_window->_window->_maxx,
            map_window->_window->_maxy);
  // clang-format on

  int cur_x = 0;
  int cur_y = 0;
  int max_x = map_window->_window->_maxx - map_window->_window->_begx;
  int max_y = (map_window->_window->_maxy - map_window->_window->_begy);
  LOG_DEBUG("max_x: %d and max_y: %d", max_x, max_y);

  // These are the four corners
  mvwaddch(map_window->_window, 0, 0, '+');
  mvwaddch(map_window->_window, 0, max_x, '+');
  mvwaddch(map_window->_window, max_y, 0, '+');
  mvwaddch(map_window->_window, max_y, max_x, '+');

  // from top-left to top-right
  cur_x = 0;
  while (cur_x < max_x - 1) {
    cur_x++;
    mvwaddch(map_window->_window, 0, cur_x, '-');
  }

  // from top-left to bottom-left
  cur_y = 0;
  wmove(map_window->_window, 0, 0);
  while (cur_y < max_y - 1) {
    cur_y++;
    mvwaddch(map_window->_window, cur_y, 0, '|');
  }

  // from bottom_left to bottom_right
  cur_x = 0;
  wmove(map_window->_window, max_y, cur_x);
  while (cur_x < max_x - 1) {
    cur_x++;
    mvwaddch(map_window->_window, max_y, cur_x, '-');
  }

  // from bottom_right to top-right
  cur_y = 0;
  wmove(map_window->_window, 0, max_x);
  while (cur_y < max_y - 1) {
    cur_y++;
    mvwaddch(map_window->_window, cur_y, max_x, '|');
  }

  // now draw the name of the map (for now it is hardcoded)
  // TODO: take the name of the map from the map object
  const char *map_name = "[ Rimini Centro ]";
  uint32_t    map_name_length = strlen(map_name);
  mvwprintw(map_window->_window, 0, max_x - 1 - map_name_length, "%s", map_name);
  wrefresh(map_window->_window);
}

void map_window_draw(MapWindow *map_window) {
  touchwin(map_window->_window);
  map_window_draw_borders(map_window);
  map_wdraw(map_window->_map, map_window->_inner_window);
}

inline UiPoint *map_window_get_ui_point(MapWindow *map_window) {
  return map_window->_ui_point;
}

inline int map_window_get_lines(MapWindow *map_window) {
  return map_window->_lines;
}

inline int map_window_get_cols(MapWindow *map_window) {
  return map_window->_cols;
}

void map_window_free(MapWindow *map_window) {
  delwin(map_window->_window);
  delwin(map_window->_inner_window);
  ui_point_free(map_window->_ui_point);
  free(map_window);
}

