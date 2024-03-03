#include "entity.h"
#include "item.h"
#include "logger.h"
#include "point.h"
#include "utils.h"
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <stdio.h>
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

bool entity_can_move(Entity *ent) {
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

char *entity_to_string(Entity *entity) {
  // Better safe than sorry I guess
  char *ret = calloc(strlen(entity->_name) + 512, sizeof(char));
  sprintf(ret, "%d:%c:%d:%d:%s", entity->_lp, entity_type_to_char(entity->_type), point_get_x(entity->_coords),
          point_get_y(entity->_coords), entity->_name);
  return ret;
}

Entity *entity_from_string(const char *input_string) {
  uint32_t starting_lp;
  char     type;
  char    *name = calloc(512, sizeof(char));
  uint32_t x;
  uint32_t y;

  LOG_INFO("Recreating entity from string: '%s'", input_string);

  sscanf(input_string, "%d:%c:%d:%d:%[^\0]s", &starting_lp, &type, &x, &y, name);
  EntityType final_type = HUMAN;
  switch (type) {
    case '@':
      final_type = HUMAN;
      break;
    case '&':
      final_type = INHUMAN;
      break;
    case '%':
      final_type = TREE;
      break;
    case '^':
      final_type = MOUNTAIN;
      break;
    case '#':
      final_type = ANIMAL;
      break;
    default:
      final_type = HUMAN;
      break;
  }

  Entity *ret = entity_new(starting_lp, final_type, name, x, y);
  free(name);

  return ret;
}

typedef struct MapKey {
  char  *str;
  size_t len;
} MapKey;

MapKey *map_key_new(const char *input) {
  MapKey *map_key = calloc(1, sizeof(MapKey));
  map_key->str = strdup(input);
  map_key->len = strlen(map_key->str);

  return map_key;
}

void map_key_free(MapKey *map_key) {
  free(map_key->str);
  free(map_key);
  map_key = nullptr;
}

void entity_serialize(Entity *ent, msgpack_sbuffer *buffer) {
  MapKey *k_current_lp = map_key_new("current_lp");
  MapKey *k_starting_lp = map_key_new("starting_lp");
  MapKey *k_type = map_key_new("type");
  MapKey *k_name = map_key_new("name");
  MapKey *k_coords = map_key_new("coords");
  MapKey *k_inventory = map_key_new("inventory");

  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  msgpack_pack_map(&packer, 6);

  msgpack_pack_str_with_body(&packer, k_current_lp->str, k_current_lp->len);
  msgpack_pack_uint64(&packer, ent->_lp);

  msgpack_pack_str_with_body(&packer, k_starting_lp->str, k_starting_lp->len);
  msgpack_pack_uint64(&packer, ent->_starting_lp);

  msgpack_pack_str_with_body(&packer, k_type->str, k_type->len);
  msgpack_pack_uint8(&packer, ent->_type);

  msgpack_pack_str_with_body(&packer, k_name->str, k_name->len);
  msgpack_pack_str_with_body(&packer, ent->_name, strlen(ent->_name));

  msgpack_pack_str_with_body(&packer, k_coords->str, k_coords->len);
  msgpack_pack_array(&packer, 2);
  msgpack_pack_uint32(&packer, point_get_x(ent->_coords));
  msgpack_pack_uint32(&packer, point_get_y(ent->_coords));

  msgpack_pack_str_with_body(&packer, k_inventory->str, k_inventory->len);
  msgpack_pack_array(&packer, 0);

  map_key_free(k_current_lp);
  map_key_free(k_starting_lp);
  map_key_free(k_type);
  map_key_free(k_name);
  map_key_free(k_coords);
  map_key_free(k_inventory);
}

char entity_type_to_char(EntityType entity_type) {
  char ret = '.';
  switch (entity_type) {
    case HUMAN:
      ret = '@';
      break;
    case ANIMAL:
      ret = '#';
      break;
    case INHUMAN:
      ret = '&';
      break;
    case TREE:
      ret = '%';
      break;
    case MOUNTAIN:
      ret = '^';
      break;
  }

  return ret;
}

const char *entity_type_to_string(EntityType entity_type) {
  switch (entity_type) {
    case HUMAN:
      return "Human";
    case ANIMAL:
      return "Animal";
    case INHUMAN:
      return "Inhuman";
    default:
      return "Unknown";
  }
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
  if (*(entity_get_entity_type(entity)) == INHUMAN && entity_is_dead(entity)) {
    LOG_INFO("Resurrecting '%s'", entity_get_name(entity));
    entity->_lp = entity->_starting_lp;
  }
}

Point *entity_get_coords(Entity *entity) {
  return entity->_coords;
}

bool entity_is_alive(Entity *entity) {
  return entity->_lp > 0;
}

bool entity_is_dead(Entity *entity) {
  return !entity_is_alive(entity);
}

EntityType *entity_get_entity_type(Entity *entity) {
  return &(entity->_type);
}

inline const char *entity_get_name(Entity *entity) {
  return entity->_name;
}

inline uint32_t entity_get_life_points(Entity *entity) {
  return entity->_lp;
}

inline uint32_t entity_get_starting_life_points(Entity *entity) {
  return entity->_starting_lp;
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
  bool   need_cleaning = false;
  size_t item_index = -1;
  size_t total_items = entity_inventory_count(entity);
  LOG_INFO("Removing item '%s' from '%s'", item_name, entity_get_name(entity));

  for (size_t i = 0; i < total_items; i++) {
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

size_t entity_inventory_count(Entity *entity) {
  size_t count = 0;
  Item  *current_item = entity->_inventory[count];
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

Item **entity_inventory_filter(Entity *entity, bool (*filter_function)(Item *), ssize_t *items_found) {
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

inline Item **entity_inventory_get(Entity *entity) {
  return entity->_inventory;
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

void entity_free(Entity *entity) {
  free(entity->_name);
  point_free(entity->_coords);
  entity_inventory_clear(entity);
  free(entity->_inventory);
  free(entity);
}
