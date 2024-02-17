#include "map.h"
#include "entity.h"
#include "item.h"
#include "logger.h"
#include "point.h"
#include "utils.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

struct Map {
  uint32_t _x_size;
  uint32_t _y_size;
  uint32_t _last_index;
  uint32_t _entities_size;
  uint32_t _items_size;
  Entity **_entities;
  Item   **_items;
};

Map *map_new(uint32_t x_size, uint32_t y_size, uint32_t max_entities) {
  Map *ret = calloc(1, sizeof(Map));
  ret->_x_size = x_size;
  ret->_y_size = y_size;
  ret->_last_index = 0;
  ret->_entities_size = max_entities;

  ret->_entities = calloc(max_entities, sizeof(Entity *));
  for (uint32_t i = 0; i < max_entities; i++) {
    ret->_entities[i] = nullptr;
  }

  ret->_items_size = 0;
  ret->_items = nullptr;

  return ret;
}

uint32_t map_count_items(Map *map) {
  return map->_items_size;
}

bool map_contains_item(Map *map, const char *item_name) {
  for (size_t i = 0; i < map->_items_size; i++) {
    if (strings_equal(item_get_name(map->_items[i]), item_name)) {
      return true;
    }
  }

  return false;
}

void map_add_item(Map *map, Item *item, uint32_t x, uint32_t y) {
  if (map_contains_item(map, item_get_name(item))) {
    return;
  }

  map->_items_size++;
  map->_items = realloc(map->_items, map->_items_size * sizeof(Item *));
  map->_items[map->_items_size - 1] = item;
  item_clear_coords(item);
  item_set_coords(item, x, y);
}

Item *map_get_item(Map *map, const char *item_name) {
  Item *ret = nullptr;
  for (size_t i = 0; i < map->_items_size; i++) {
    if (strings_equal(item_get_name(map->_items[i]), item_name)) {
      ret = map->_items[i];
      break;
    }
  }

  return ret;
}

void map_remove_item(Map *map, const char *name) {
  if (!map_contains_item(map, name)) {
    return;
  }

  size_t item_index = -1;

  for (size_t i = 0; i < map->_items_size; i++) {
    if (strings_equal(item_get_name(map->_items[i]), name)) {
      item_index = i;
      item_free(map->_items[i]);
    } else if (item_index != -1) {
      map->_items[i - 1] = map->_items[i];
    }
  }

  if (item_index != -1) {
    map->_items_size--;
    map->_items = realloc(map->_items, map->_items_size * sizeof(Item *));
  }
}

Map *map_from_string(const char *input) {
  uint32_t x_size;
  uint32_t y_size;
  uint32_t count;
  uint32_t entities_size;
  sscanf(input, "%d:%d:%d:%d", &x_size, &y_size, &count, &entities_size);

  return map_new(x_size, y_size, entities_size);
}

void map_dump_to_file(Map *map, const char *path) {
  FILE *output_file = fopen(path, "wb");

  fprintf(output_file, "%d:%d:%d:%d\n", map->_x_size, map->_y_size, map->_last_index, map->_entities_size);

  for (uint32_t i = 0; i < map->_last_index; i++) {
    fprintf(output_file, "%s\n", entity_to_string(map->_entities[i]));
  }

  fclose(output_file);
}

Entity **map_get_all_entities(Map *map) {
  return map->_entities;
}

Entity **map_filter_entities(Map *map, bool (*filter_function)(Entity *), ssize_t *nb_results) {
  Entity **result = nullptr;
  *nb_results = 0;

  for (ssize_t i = 0; i < map->_last_index; i++) {
    if (filter_function(map->_entities[i])) {
      result = realloc(result, (*nb_results + 1) * sizeof(Entity *));
      result[*nb_results] = map->_entities[i];
      (*nb_results)++;
    }
  }

  return result;
}

int map_count_entities(Map *map) {
  int count = 0;
  for (uint32_t i = 0; i < map->_entities_size; i++) {
    if (map->_entities[i] != nullptr) {
      count++;
    }
  }

  return count;
}

void map_add_entity(Map *map, Entity *entity) {
  Point *coords = entity_get_coords(entity);
  if (!map_is_tile_free(map, point_get_x(coords), point_get_y(coords))) {
    return;
  }

  if (map->_last_index < map->_entities_size) {
    map->_entities[map->_last_index] = entity;
    map->_last_index++;
  }
}

Entity *map_get_entity(Map *map, const char *name) {
  Entity *ret = nullptr;
  for (uint32_t i = 0; i < map->_last_index; i++) {
    Entity *current_entity = map->_entities[i];
    if (strings_equal(entity_get_name(current_entity), name)) {
      ret = current_entity;
      break;
    }
  }

  return ret;
}

int map_get_index_of_entity(Map *map, const char *name) {
  int32_t index = -1;
  for (int32_t i = 0; i < map->_last_index; i++) {
    if (strings_equal(entity_get_name(map->_entities[i]), name)) {
      index = i;
      break;
    }
  }

  return index;
}

void map_update_index(Map *map) {
  uint32_t count = 0;
  for (uint32_t i = 0; i < map->_entities_size; i++) {
    if (map->_entities[i] == nullptr) {
      break;
    }

    count++;
  }

  map->_last_index = count;
}

void map_remove_entity(Map *map, const char *name) {
  if (!map_contains_entity(map, name)) {
    return;
  }

  uint32_t removed_index = 0;
  for (; removed_index < map->_last_index; removed_index++) {
    Entity *current_entity = map->_entities[removed_index];
    if (strings_equal(entity_get_name(current_entity), name)) {
      map->_entities[removed_index] = nullptr;
      entity_free(current_entity);
      break;
    }
  }

  // Now reorder all the heap!
  for (uint32_t i = removed_index + 1; i < map->_last_index; i++) {
    if (map->_entities[i] == nullptr) {
      map->_last_index = i;
      break;
    }
    map->_entities[i - 1] = map->_entities[i];
    map->_entities[i] = nullptr;
  }

  map_update_index(map);
}

bool map_contains_entity(Map *map, const char *name) {
  Entity *ret = map_get_entity(map, name);
  return ret != nullptr ? 1 : 0;
}

bool map_is_tile_free(Map *map, uint32_t x, uint32_t y) {
  bool is_free = true;
  for (uint32_t i = 0; i < map->_last_index; i++) {
    Point *point = entity_get_coords(map->_entities[i]);
    if (point_get_x(point) == x && point_get_y(point) == y) {
      is_free = false;
      break;
    }
  }

  return is_free;
}

void map_draw(Map *map) {
  map_fdraw(map, stdout);
}

char **generate_matrix(Map *map) {
  char **matrix;

  matrix = calloc(map->_x_size, sizeof(char *));
  for (uint32_t i = 0; i < map->_x_size; i++) {
    matrix[i] = calloc(map->_y_size, sizeof(char));
  }

  // Fill the matrix with dots
  for (int x = 0; x < map->_x_size; x++) {
    for (int y = 0; y < map->_y_size; y++) {
      matrix[x][y] = '.';
    }
  }

  // Now take all the entities from the map and draw them in the matrix
  for (uint32_t i = 0; i < map->_last_index; i++) {
    Entity *current_entity = map->_entities[i];
    Point  *point = entity_get_coords(current_entity);
    matrix[point_get_x(point)][point_get_y(point)] = entity_type_to_char(*entity_get_entity_type(current_entity));
  }

  return matrix;
}

void free_matrix(Map *map, char **matrix) {
  for (int i = 0; i < map->_x_size; i++) {
    free(matrix[i]);
  }

  free(matrix);
}

void map_fdraw(Map *map, FILE *output) {
  char **matrix = generate_matrix(map);

  // Each `y` is a line, `x` is a column
  for (int y = 0; y < map->_y_size; y++) {
    for (int x = 0; x < map->_x_size; x++) {
      fprintf(output, "%c", matrix[x][y]);
    }

    fprintf(output, "\n");
  }

  free_matrix(map, matrix);
}

void map_wdraw(Map *map, WINDOW *window) {
  wmove(window, 0, 0);
  int max_x = window->_maxx - window->_begx;
  int max_y = window->_maxy - window->_begy;

  if (max_y < map->_y_size) {
    LOG_ERROR("Cannot draw map, screen (y: %d < %d) is too small!", max_y, map->_y_size);
    return;
  }

  if (max_x < map->_x_size) {
    LOG_ERROR("Cannot draw map, screen (x: %d < %d) is too small!", max_x, map->_x_size);
    return;
  }

  LOG_INFO("Drawing map on a %dx%d window", max_x, max_y);

  char **matrix = generate_matrix(map);

  for (int y = 0; y < map->_y_size; y++) {
    for (int x = 0; x < map->_x_size; x++) {
      wmove(window, y, x);
      if (matrix[x][y] == '@') {
        wattron(window, COLOR_PAIR(1));
      } else if (matrix[x][y] == '^') {
        wattron(window, COLOR_PAIR(2));
      } else if (matrix[x][y] == '&') {
        wattron(window, COLOR_PAIR(3));
      } else if (matrix[x][y] == '%') {
        wattron(window, COLOR_PAIR(4));
      } else if (matrix[x][y] == '#') {
        wattron(window, COLOR_PAIR(5));
      }

      wprintw(window, "%c", matrix[x][y]);
      wattroff(window, COLOR_PAIR(1));
    }
  }

  wmove(window, 0, 0);
  wrefresh(window);

  free_matrix(map, matrix);
  return;
}

MapBoundaries map_get_boundaries(Map *map) {
  MapBoundaries boundaries;
  boundaries.x = map->_x_size;
  boundaries.y = map->_y_size;

  return boundaries;
}

void map_free(Map *map) {
  for (uint32_t i = 0; i < map->_last_index; i++) {
    entity_free(map->_entities[i]);
  }

  if (map->_items != nullptr) {
    for (size_t i = 0; i < map->_items_size; i++) {
      item_free(map->_items[i]);
    }
    free(map->_items);
  }

  free(map->_entities);
  free(map);

  map = nullptr;
}

