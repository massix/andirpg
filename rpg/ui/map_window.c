#include "map_window.h"
#include "logger.h"
#include "map.h"
#include "ui_point.h"
#include <ncurses.h>
#include <stdlib.h>

struct MapWindow {
  WINDOW  *_window;
  Map     *_map; // This map is owned by an engine, we *must not* free it!
  UiPoint *_ui_point;
  int      _lines;
  int      _cols;
};

MapWindow *map_window_new(Map *map, int lines, int cols, int x, int y) {
  MapWindow *ret = calloc(1, sizeof(MapWindow));
  LOG_INFO("Creating map window (%dx%d)", lines, cols);

  ret->_ui_point = ui_point_new(x, y);
  ret->_cols = cols;
  ret->_lines = lines;

  // Before you ask: no, it is not a bug. NCurses has `y` and `x` reversed.
  ret->_window = newwin(ret->_cols, ret->_lines, y, x);
  ret->_map = map;

  return ret;
}

inline WINDOW *map_window_get_ncurses_window(MapWindow *map_window) {
  return map_window->_window;
}

void map_window_draw(MapWindow *map_window) {
  map_wdraw(map_window->_map, map_window->_window);
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
  ui_point_free(map_window->_ui_point);
  free(map_window);
}

