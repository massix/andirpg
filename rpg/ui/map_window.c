#include "map_window.h"
#include "boxed_window.h"
#include "entity.h"
#include "logger.h"
#include "map.h"
#include "point.h"
#include "tile.h"
#include "ui_point.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>

struct MapWindow {
  BoxedWindow *_window;
  Map         *_map; // This map is owned by an engine, we *must not* free it!
  UiPoint     *_ui_point;
};

bool map_window_check_win_size(Map *map, int lines, int cols) {
  MapBoundaries map_boundaries = map_get_boundaries(map);

  bool lines_ok = lines > map_boundaries.y + 2;
  bool cols_ok = cols > map_boundaries.x + 2;

  return lines_ok && cols_ok;
}

MapWindow *map_window_new(Map *map, int lines, int cols, int x, int y) {
  if (!map_window_check_win_size(map, lines, cols)) {
    LOG_CRITICAL("Winsize (%dx%d) is too small to draw map and border", cols, lines);
    return nullptr;
  }

  MapWindow *ret = calloc(1, sizeof(MapWindow));
  LOG_INFO("Creating map window (%dx%d)", lines, cols);

  struct BoxedWindowOptions bwo;
  boxed_window_options_default(&bwo);

  ret->_ui_point = ui_point_new(x, y);

  ret->_map = map;
  ret->_window = boxed_window_new(&bwo, map_get_name(map), lines, cols, y, x, nullptr);

  return ret;
}

inline WINDOW *map_window_get_ncurses_window(MapWindow *map_window) {
  return nullptr;
}

char **map_window_generate_matrix(MapWindow *mpw) {
  char        **matrix;
  MapBoundaries boundaries = map_get_boundaries(mpw->_map);
  uint32_t      entities_size = map_count_entities(mpw->_map);
  Entity      **all_entities = map_get_all_entities(mpw->_map);

  matrix = calloc(boundaries.x, sizeof(char *));
  for (uint32_t i = 0; i < boundaries.x; i++) {
    matrix[i] = calloc(boundaries.y, sizeof(char));
  }

  // Fill the matrix with characters depending on the tile type
  for (int x = 0; x < boundaries.x; x++) {
    for (int y = 0; y < boundaries.y; y++) {
      matrix[x][y] = tile_get_tile_kind(map_get_tile(mpw->_map, x, y));
    }
  }

  // Now take all the entities from the map and draw them in the matrix
  for (uint32_t i = 0; i < entities_size; i++) {
    Entity *current_entity = all_entities[i];
    Point  *point = entity_get_coords(current_entity);
    matrix[point_get_x(point)][point_get_y(point)] = entity_type_to_char(*entity_get_entity_type(current_entity));
  }

  return matrix;
}

void map_window_free_matrix(MapWindow *mpw, char **matrix) {
  MapBoundaries boundaries = map_get_boundaries(mpw->_map);
  for (int i = 0; i < boundaries.y; i++) {
    free(matrix[i]);
  }

  free(matrix);
}

void map_window_draw(MapWindow *map_window) {
  MapBoundaries boundaries = map_get_boundaries(map_window->_map);
  boxed_window_draw(map_window->_window);

  WINDOW *target = boxed_window_get_inner_window(map_window->_window);
  char  **matrix = map_window_generate_matrix(map_window);

  wclear(target);

  for (int y = 0; y < boundaries.y; y++) {
    for (int x = 0; x < boundaries.x; x++) {
      if (matrix[x][y] == '@') {
        wattron(target, COLOR_PAIR(1));
      } else if (matrix[x][y] == '^') {
        wattron(target, COLOR_PAIR(2));
      } else if (matrix[x][y] == '&') {
        wattron(target, COLOR_PAIR(3));
      } else if (matrix[x][y] == '%') {
        wattron(target, COLOR_PAIR(4));
      } else if (matrix[x][y] == '#') {
        wattron(target, COLOR_PAIR(5));
      }
      mvwprintw(target, y, x, "%c", matrix[x][y]);
      wattroff(target, COLOR_PAIR(1));
    }
  }

  wnoutrefresh(target);

  map_window_free_matrix(map_window, matrix);
}

inline UiPoint *map_window_get_ui_point(MapWindow *map_window) {
  return map_window->_ui_point;
}

inline int map_window_get_lines(MapWindow *map_window) {
  return boxed_window_get_lines(map_window->_window);
}

inline int map_window_get_cols(MapWindow *map_window) {
  return boxed_window_get_cols(map_window->_window);
}

void map_window_free(MapWindow *map_window) {
  ui_point_free(map_window->_ui_point);
  boxed_window_free(map_window->_window);
  free(map_window);
}

