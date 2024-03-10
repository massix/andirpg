// AndiRPG - Name not final
// Copyright © 2024 Massimo Gengarelli
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "map.h"
#include "entity.h"
#include "item.h"
#include "logger.h"
#include "point.h"
#include "serde.h"
#include "tile.h"
#include "utils.h"
#include <assert.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <msgpack/unpack.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct Map {
  uint32_t _x_size;
  uint32_t _y_size;
  uint32_t _last_index;
  uint32_t _entities_size;
  uint32_t _items_size;
  char    *_name;
  Entity **_entities;
  Item   **_items;
  Tile   **_tiles;
};

Map *map_new(uint32_t x_size, uint32_t y_size, uint32_t max_entities, char const *name) {
  Map *ret = calloc(1, sizeof(Map));
  ret->_x_size = x_size;
  ret->_y_size = y_size;
  ret->_last_index = 0;
  ret->_entities_size = max_entities;
  ret->_name = strdup(name);

  unsigned long tiles_size = (unsigned long)x_size * y_size;

  // Create the tiles array, store all the xs then all the ys
  ret->_tiles = calloc(tiles_size, sizeof(Tile **));
  uint32_t current_index = 0;
  for (uint32_t x = 0; x < x_size; x++) {
    for (uint32_t y = 0; y < y_size; y++) {
      ret->_tiles[current_index] = tile_new(GRASS, x, y);
      current_index++;
    }
  }

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

  ssize_t item_index = -1;

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

inline char const *map_get_name(Map const *map) {
  return map->_name;
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

Map *map_deserialize(msgpack_object_map *msgpack_map) {
  LOG_INFO("Unmarshalling map", 0);

  // Check the validity of the map before starting the Unmarshalling process
  assert(msgpack_map->size == 8);
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "x_size");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "y_size");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "max_entities");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "last_index");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_STR, "name");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_ARRAY, "entities");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_ARRAY, "items");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_ARRAY, "tiles");

  msgpack_object_array const *items = serde_map_get(msgpack_map, MSGPACK_OBJECT_ARRAY, "items");
  msgpack_object_array const *entities = serde_map_get(msgpack_map, MSGPACK_OBJECT_ARRAY, "entities");
  msgpack_object_array const *tiles = serde_map_get(msgpack_map, MSGPACK_OBJECT_ARRAY, "tiles");
  msgpack_object_str const   *name = serde_map_get(msgpack_map, MSGPACK_OBJECT_STR, "name");

  Map *map = calloc(1, sizeof(Map));

  map->_x_size = *(uint32_t *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "x_size");
  map->_y_size = *(uint32_t *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "y_size");
  map->_entities_size = *(uint32_t *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "max_entities");
  map->_last_index = *(uint32_t *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "last_index");
  map->_name = malloc(name->size);
  memcpy(map->_name, name->ptr, name->size);
  map->_items_size = items->size;

  map->_entities = calloc(map->_entities_size, sizeof(Entity *));
  for (uint i = 0; i < map->_entities_size; i++) {
    map->_entities[i] = nullptr;
  }

  for (uint i = 0; i < entities->size; i++) {
    msgpack_object_map entity_map = entities->ptr[i].via.map;
    map->_entities[i] = entity_deserialize(&entity_map);
  }

  // +1 because we need to allocate the nullptr
  map->_items = calloc(items->size + 1, sizeof(Item *));
  map->_items[items->size] = nullptr;
  for (uint i = 0; i < items->size; i++) {
    msgpack_object_map item_map = items->ptr[i].via.map;
    map->_items[i] = item_deserialize(&item_map);
  }

  map->_tiles = calloc(tiles->size, sizeof(Tile *));
  for (uint i = 0; i < tiles->size; i++) {
    map->_tiles[i] = tile_deserialize(&tiles->ptr[i].via.map);
  }

  return map;
}

void map_serialize(Map *map, msgpack_sbuffer *buffer) {
  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  // Map is a dictionary
  msgpack_pack_map(&packer, 8);

  serde_pack_str(&packer, "x_size");
  msgpack_pack_uint32(&packer, map->_x_size);

  serde_pack_str(&packer, "y_size");
  msgpack_pack_uint32(&packer, map->_y_size);

  serde_pack_str(&packer, "name");
  serde_pack_str(&packer, map->_name);

  serde_pack_str(&packer, "max_entities");
  msgpack_pack_uint32(&packer, map->_entities_size);

  serde_pack_str(&packer, "last_index");
  msgpack_pack_uint32(&packer, map->_last_index);

  serde_pack_str(&packer, "entities");
  msgpack_pack_array(&packer, map_count_entities(map));
  for (uint32_t i = 0; i < map_count_entities(map); i++) {
    entity_serialize(map->_entities[i], buffer);
  }

  serde_pack_str(&packer, "items");
  msgpack_pack_array(&packer, map_count_items(map));
  for (uint32_t i = 0; i < map_count_items(map); i++) {
    item_serialize(map->_items[i], buffer);
  }

  serde_pack_str(&packer, "tiles");
  unsigned long total_tiles = (unsigned long)map->_x_size * map->_y_size;
  msgpack_pack_array(&packer, total_tiles);
  for (uint32_t i = 0; i < total_tiles; i++) {
    tile_serialize(map->_tiles[i], buffer);
  }
}

Tile *map_modify_tile(Map const *map, uint32_t x, uint32_t y) {
  if (x >= map->_x_size || y >= map->_y_size) {
    return nullptr;
  }

  return map->_tiles[y + ((unsigned long)x * map->_y_size)];
}

inline Tile const *map_get_tile(Map const *map, uint32_t x, uint32_t y) {
  return (Tile const *)map_modify_tile(map, x, y);
}

inline void map_set_tile_properties(Map const *map, uint32_t x, uint32_t y, TileProperties const *tile_props) {
  Tile *tile_at = map_modify_tile(map, x, y);
  if (tile_at != nullptr) {
    tile_set_base_light(tile_at, tile_props->base_light);
    tile_set_inside(tile_at, tile_props->inside);
    tile_set_traversable(tile_at, tile_props->traversable);
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

  for (uint32_t i = 0; i < (unsigned long)map->_x_size * map->_y_size; i++) {
    tile_free(map->_tiles[i]);
  }

  free(map->_tiles);
  free(map->_entities);
  free(map->_name);
  free(map);
}

