#include "map.h"
#include "entity.h"
#include "item.h"
#include "point.h"
#include "utils.h"
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <msgpack/unpack.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

MapBoundaries map_get_boundaries(Map *map) {
  MapBoundaries boundaries;
  boundaries.x = map->_x_size;
  boundaries.y = map->_y_size;

  return boundaries;
}

// uint32_t _x_size;
// uint32_t _y_size;
// uint32_t _last_index;
// uint32_t _entities_size;
// uint32_t _items_size;
// Entity **_entities;
// Item   **_items;

void map_serialize(Map *map, msgpack_sbuffer *buffer) {
  const char *x_size_key = "x_size";
  const char *y_size_key = "y_size";
  const char *max_entities_key = "max_entities";
  const char *last_index_key = "last_index";
  const char *entities_key = "entities";
  const char *items_key = "items";

  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  // Map is a dictionary
  msgpack_pack_map(&packer, 6);

  msgpack_pack_str_with_body(&packer, x_size_key, strlen(x_size_key));
  msgpack_pack_uint32(&packer, map->_x_size);

  msgpack_pack_str_with_body(&packer, y_size_key, strlen(y_size_key));
  msgpack_pack_uint32(&packer, map->_y_size);

  msgpack_pack_str_with_body(&packer, max_entities_key, strlen(max_entities_key));
  msgpack_pack_uint32(&packer, map->_entities_size);

  msgpack_pack_str_with_body(&packer, last_index_key, strlen(last_index_key));
  msgpack_pack_uint32(&packer, map->_last_index);

  msgpack_pack_str_with_body(&packer, entities_key, strlen(entities_key));
  msgpack_pack_array(&packer, map_count_entities(map));
  for (uint32_t i = 0; i < map_count_entities(map); i++) {
    entity_serialize(map->_entities[i], buffer);
  }

  msgpack_pack_str_with_body(&packer, items_key, strlen(items_key));
  msgpack_pack_array(&packer, map_count_items(map));
  for (uint32_t i = 0; i < map_count_items(map); i++) {
    item_serialize(map->_items[i], buffer);
  }
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

