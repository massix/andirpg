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

#include "entity.h"
#include "item.h"
#include "logger.h"
#include "point.h"
#include "serde.h"
#include "utils.h"
#include <assert.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// This can be a compile-time constant?
#ifndef MAX_INVENTORY_SIZE
#define MAX_INVENTORY_SIZE 1024
#endif

struct Entity {
  uint32_t   _lp;
  uint32_t   _starting_lp;
  EntityType _type;
  char      *_name;
  Point     *_coords;
  Item     **_inventory;
};

Entity *entity_new(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y) {
  LOG_DEBUG("Creating new entity '%s'", name);

  Entity *ret = calloc(1, sizeof(Entity));
  ret->_lp = starting_lp > 0 ? starting_lp : 1;
  ret->_starting_lp = ret->_lp;
  ret->_type = type;
  ret->_name = strdup(name);
  ret->_inventory = calloc(1, sizeof(Item *));
  ret->_inventory[0] = nullptr;
  ret->_coords = point_new(start_x, start_y);
  return ret;
}

Entity *entity_deserialize(msgpack_object_map const *map) {
  LOG_INFO("Unmarshalling entity", 0);
  LOG_INFO("Validating map", 0);

  assert(map->size == 6);
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "current_lp");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "starting_lp");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "type");
  serde_map_assert(map, MSGPACK_OBJECT_STR, "name");
  serde_map_assert(map, MSGPACK_OBJECT_ARRAY, "coords");
  serde_map_assert(map, MSGPACK_OBJECT_ARRAY, "inventory");

  uint32_t   current_lp = *(uint32_t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "current_lp");
  uint32_t   starting_lp = *(uint32_t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "starting_lp");
  EntityType type = *(EntityType *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "type");

  msgpack_object_array const *coords_array = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "coords");
  assert(coords_array->size == 2);

  Point *coords = point_new(coords_array->ptr[0].via.u64, coords_array->ptr[1].via.u64);

  Entity *entity = calloc(1, sizeof(Entity));
  entity->_starting_lp = starting_lp;
  entity->_type = type;
  entity->_lp = current_lp;
  entity->_coords = coords;

  msgpack_object_str const *name_ptr = serde_map_get(map, MSGPACK_OBJECT_STR, "name");

  entity->_name = calloc(name_ptr->size, sizeof(char));
  memcpy(entity->_name, name_ptr->ptr, name_ptr->size);

  msgpack_object_array const *inventory = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "inventory");

  entity->_inventory = calloc(inventory->size + 1 /* for the null pointer */, sizeof(Item *));
  entity->_inventory[inventory->size] = nullptr;
  for (uint i = 0; i < inventory->size; i++) {
    entity->_inventory[i] = item_deserialize(&(inventory->ptr[i].via.map));
  }

  return entity;
}

void entity_free(Entity *entity) {
  free(entity->_name);
  point_free(entity->_coords);
  entity_inventory_clear(entity);
  free(entity->_inventory);
  free(entity);
}

void entity_serialize(Entity const *ent, msgpack_sbuffer *buffer) {
  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  msgpack_pack_map(&packer, 6);

  serde_pack_str(&packer, "current_lp");
  msgpack_pack_uint64(&packer, ent->_lp);

  serde_pack_str(&packer, "starting_lp");
  msgpack_pack_uint64(&packer, ent->_starting_lp);

  serde_pack_str(&packer, "type");
  msgpack_pack_uint8(&packer, ent->_type);

  serde_pack_str(&packer, "name");
  serde_pack_str(&packer, ent->_name);

  serde_pack_str(&packer, "coords");
  msgpack_pack_array(&packer, 2);
  msgpack_pack_uint32(&packer, point_get_x(ent->_coords));
  msgpack_pack_uint32(&packer, point_get_y(ent->_coords));

  serde_pack_str(&packer, "inventory");
  uint32_t inventory_count = entity_inventory_count(ent);
  msgpack_pack_array(&packer, inventory_count);
  for (uint32_t i = 0; i < inventory_count; i++) {
    item_serialize(ent->_inventory[i], buffer);
  }
}

inline uint32_t entity_get_life_points(Entity const *entity) {
  return entity->_lp;
}

inline uint32_t entity_get_starting_life_points(Entity const *entity) {
  return entity->_starting_lp;
}

EntityType entity_get_entity_type(Entity const *entity) {
  return entity->_type;
}

inline const char *entity_get_name(Entity const *entity) {
  return entity->_name;
}

Point const *entity_get_coords(Entity const *entity) {
  return entity->_coords;
}

bool entity_can_move(Entity const *ent) {
  bool ret = true;
  switch (ent->_type) {
    case HUMAN:
    case ANIMAL:
    case INHUMAN:
      ret = true;
      break;
    case TREE:
    case MOUNTAIN:
      ret = false;
      break;
  }

  return ret && entity_is_alive(ent);
}

bool entity_is_alive(Entity const *entity) {
  return entity->_lp > 0;
}

bool entity_is_dead(Entity const *entity) {
  return !entity_is_alive(entity);
}

void entity_move(Entity *entity, uint32_t delta_x, uint32_t delta_y) {
  if (entity_can_move(entity)) {
    point_set_x(entity->_coords, point_get_x(entity->_coords) + delta_x);
    point_set_y(entity->_coords, point_get_y(entity->_coords) + delta_y);
  }
}

void entity_hurt(Entity *entity, uint32_t life_points) {
  if (life_points > entity->_lp) {
    entity->_lp = 0;
  } else {
    entity->_lp -= life_points;
  }
}

void entity_heal(Entity *entity, uint32_t life_points) {
  if (entity->_lp > 0) {
    entity->_lp = min(entity->_starting_lp, entity->_lp + life_points);
  }
}

void entity_resurrect(Entity *entity) {
  if (entity_get_entity_type(entity) == INHUMAN && entity_is_dead(entity)) {
    LOG_INFO("Resurrecting '%s'", entity_get_name(entity));
    entity->_lp = entity->_starting_lp;
  }
}

size_t entity_inventory_count(Entity const *entity) {
  size_t count = 0;

  Item const *current_item = entity->_inventory[count];
  while (current_item != nullptr) {
    current_item = entity->_inventory[++count];
  }

  return count;
}

void entity_inventory_add_item(Entity *entity, Item *item) {
  size_t current_count = entity_inventory_count(entity) + 1; // include the nullptr
  LOG_INFO("Adding item '%s' to '%s'", item_get_name(item), entity_get_name(entity));

  // current_count + 1 adds a new "slot"
  entity->_inventory = realloc(entity->_inventory, (current_count + 1) * sizeof(Item *));

  // current_count - 1 is the old nullptr
  entity->_inventory[current_count - 1] = item;
  entity->_inventory[current_count] = nullptr;
}

// When removing an item from the inventory, we will replace the empty slot with the last item of the inventory
// Example:
// Inventory contains:
// 0. a potion
// 1. a sword
// 2. a dagger
// 3. an armor
//
// We want to remove a sword, the new inventory will be:
// 0. a potion
// 1. an armor (was number 4, now is number 2)
// 2. a dagger
void entity_inventory_remove_item(Entity *entity, const char *item_name) {
  bool    need_cleaning = false;
  ssize_t item_index = -1;
  size_t  total_items = entity_inventory_count(entity);
  LOG_INFO("Removing item '%s' from '%s'", item_name, entity_get_name(entity));

  for (ssize_t i = 0; i < total_items; i++) {
    if (strings_equal(item_get_name(entity->_inventory[i]), item_name)) {
      LOG_DEBUG("Item found, removing", 0);
      item_index = i;
      item_free(entity->_inventory[i]);
      entity->_inventory[i] = nullptr;
      need_cleaning = i < (total_items - 1) && entity->_inventory[i + 1] != nullptr;
      break;
    }
  }

  if (need_cleaning) {
    LOG_DEBUG("Cleaning inventory after removal of item", 0);
    entity->_inventory[item_index] = entity->_inventory[total_items - 1];
    entity->_inventory[total_items - 1] = nullptr;
  }

  if (item_index != -1) {
    entity->_inventory = realloc(entity->_inventory, (total_items - 1) * sizeof(Item *));
  }
}

void entity_inventory_clear(Entity *entity) {
  size_t current_index = 0;
  Item  *pointed_item = entity->_inventory[current_index];
  LOG_DEBUG("Cleaning inventory for '%s'", entity_get_name(entity));

  while (pointed_item != nullptr) {
    item_free(pointed_item);
    pointed_item = entity->_inventory[++current_index];
  }

  entity->_inventory = realloc(entity->_inventory, 1 * sizeof(Item *));
  entity->_inventory[0] = nullptr;
}

Item **entity_inventory_filter(Entity *entity, bool (*filter_function)(Item const *), ssize_t *items_found) {
  Item **elements = calloc(entity_inventory_count(entity), sizeof(Item *));
  *items_found = 0;
  ssize_t current_index = 0;

  Item *current_item = entity->_inventory[current_index];
  while (current_item != nullptr) {
    if (filter_function(current_item)) {
      elements[*items_found] = current_item;
      (*items_found)++;
    }

    current_item = entity->_inventory[++current_index];
  }

  return elements;
}

inline Item **entity_inventory_get(Entity const *entity) {
  return entity->_inventory;
}
