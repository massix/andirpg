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
#include "perk.h"
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
#include <sys/cdefs.h>
#include <sys/types.h>

#define GENERATE_SETTER(prop_name)                                 \
  inline void entity_set_##prop_name(Entity *self, uint32_t val) { \
    self->_##prop_name = val;                                      \
  }

#define GENERATE_GETTER(rtype, prop_name)                   \
  inline rtype entity_get_##prop_name(Entity const *self) { \
    return self->_##prop_name;                              \
  }

// This can be a compile-time constant?
#ifndef MAX_INVENTORY_SIZE
#define MAX_INVENTORY_SIZE 1024
#endif

struct Entity {
  uint32_t   _lp;
  uint32_t   _starting_lp;
  uint32_t   _mental_health;
  uint32_t   _starting_mental_health;
  uint32_t   _hunger;
  uint32_t   _thirst;
  uint32_t   _tiredness;
  uint32_t   _xp;
  uint32_t   _current_level;
  uint32_t   _hearing_distance;
  uint32_t   _seeing_distance;
  EntityType _type;
  char      *_name;
  Point     *_coords;
  Item     **_inventory;
  Perk     **_perks;
};

EntityBuilder *eb_with_type(EntityBuilder *self, EntityType type) {
  self->type = type;
  return self;
}

EntityBuilder *eb_with_lp(EntityBuilder *self, uint32_t life_points) {
  self->life_points = life_points;
  return self;
}

EntityBuilder *eb_with_mh(EntityBuilder *self, uint32_t mental_health) {
  self->mental_health = mental_health;
  return self;
}

EntityBuilder *eb_with_level(EntityBuilder *self, uint32_t level) {
  self->level = level;
  return self;
}

EntityBuilder *eb_with_xp(EntityBuilder *self, uint32_t xp) {
  self->xp = xp;
  return self;
}

EntityBuilder *eb_with_hd(EntityBuilder *self, uint32_t hearing_distance) {
  self->hearing_distance = hearing_distance;
  return self;
}

EntityBuilder *eb_with_seeing_distance(EntityBuilder *self, uint32_t seeing_distance) {
  self->seeing_distance = seeing_distance;
  return self;
}

EntityBuilder *eb_with_name(EntityBuilder *self, char const *name) {
  if (self->name != nullptr) {
    free(self->name);
  }

  self->name = strdup(name);
  return self;
}

EntityBuilder *eb_with_coords(EntityBuilder *self, uint32_t x, uint32_t y) {
  self->x = x;
  self->y = y;
  return self;
}

EntityBuilder *eb_with_hunger(EntityBuilder *self, uint32_t hunger) {
  self->hunger = hunger;
  return self;
}

EntityBuilder *eb_with_thirst(EntityBuilder *self, uint32_t thirst) {
  self->thirst = thirst;
  return self;
}

EntityBuilder *eb_with_tiredness(EntityBuilder *self, uint32_t tiredness) {
  self->tiredness = tiredness;
  return self;
}

Entity *eb_build(EntityBuilder *self, bool oneshot) {
  if (self->name == nullptr) {
    panic("Cannot build an entity without a name!", EC_ENTITY_EMPTY_NAME);
  }

  Entity *ent = calloc(1, sizeof(Entity));
  ent->_lp = self->life_points;
  ent->_starting_lp = self->life_points;
  ent->_mental_health = self->mental_health;
  ent->_starting_mental_health = self->mental_health;
  ent->_hunger = self->hunger;
  ent->_thirst = self->thirst;
  ent->_tiredness = self->tiredness;
  ent->_xp = self->xp;
  ent->_current_level = self->level;
  ent->_hearing_distance = self->hearing_distance;
  ent->_seeing_distance = self->seeing_distance;
  ent->_type = self->type;
  ent->_name = strdup(self->name);
  ent->_coords = point_new(self->x, self->y);
  ent->_inventory = calloc(1, sizeof(Item *));
  ent->_inventory[0] = nullptr;
  ent->_perks = calloc(1, sizeof(Perk *));
  ent->_perks[0] = nullptr;

  if (oneshot) {
    entity_builder_free(self);
  }

  return ent;
}

// Builder for entities
EntityBuilder *entity_builder_new() {
  EntityBuilder *builder = calloc(1, sizeof(EntityBuilder));

  // Init with some default values
  builder->name = nullptr;
  builder->type = INHUMAN;
  builder->life_points = 30;
  builder->mental_health = 30;
  builder->level = 0;
  builder->xp = 0;
  builder->hearing_distance = 10;
  builder->seeing_distance = 10;
  builder->x = 0;
  builder->y = 0;
  builder->hunger = 0;
  builder->thirst = 0;
  builder->tiredness = 0;

  // Methods
  builder->with_type = &eb_with_type;
  builder->with_life_points = &eb_with_lp;
  builder->with_mental_health = &eb_with_mh;
  builder->with_level = &eb_with_level;
  builder->with_xp = &eb_with_xp;
  builder->with_hearing_distance = &eb_with_hd;
  builder->with_seeing_distance = &eb_with_seeing_distance;
  builder->with_name = &eb_with_name;
  builder->with_coords = &eb_with_coords;
  builder->with_hunger = &eb_with_hunger;
  builder->with_thirst = &eb_with_thirst;
  builder->with_tiredness = &eb_with_tiredness;
  builder->build = &eb_build;

  return builder;
}

void entity_builder_free(EntityBuilder *self) {
  free(self->name);
  free(self);
}

Entity *entity_new(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y) {
  LOG_ERROR("Trying to create an entity using old constructor", 0);
  panic("entity_new() is deprecated, please use entity_build() with the same parameters", EC_DEPRECATED_FUNCTION);

  return nullptr; // Should never arrive here
}

Entity *entity_build(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y) {
  EntityBuilder *builder = entity_builder_new();
  return builder->with_life_points(builder, starting_lp)
    ->with_type(builder, type)
    ->with_name(builder, name)
    ->with_coords(builder, start_x, start_y)
    ->build(builder, true);
}

Entity *entity_deserialize(msgpack_object_map const *map) {
  LOG_INFO("Unmarshalling entity", 0);
  LOG_INFO("Validating map", 0);

  assert(map->size == 15);

#define sma(t, f) serde_map_assert(map, MSGPACK_OBJECT_##t, f);

  sma(POSITIVE_INTEGER, "lp");
  sma(POSITIVE_INTEGER, "starting_lp");
  sma(POSITIVE_INTEGER, "mental_health");
  sma(POSITIVE_INTEGER, "starting_mental_health");
  sma(POSITIVE_INTEGER, "hunger");
  sma(POSITIVE_INTEGER, "thirst");
  sma(POSITIVE_INTEGER, "tiredness");
  sma(POSITIVE_INTEGER, "xp");
  sma(POSITIVE_INTEGER, "current_level");
  sma(POSITIVE_INTEGER, "hearing_distance");
  sma(POSITIVE_INTEGER, "seeing_distance");
  sma(POSITIVE_INTEGER, "type");
  sma(STR, "name");
  sma(ARRAY, "coords");
  sma(ARRAY, "inventory");

#define smg(t, f) t f = *(t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, #f);

  // NOLINTNEXTLINE(readability-identifier-length)
  smg(uint32_t, lp);
  smg(uint32_t, starting_lp);
  smg(uint32_t, mental_health);
  smg(uint32_t, starting_mental_health);
  smg(uint32_t, hunger);
  smg(uint32_t, thirst);
  smg(uint32_t, tiredness);
  smg(uint32_t, xp);
  smg(uint32_t, current_level);
  smg(uint32_t, hearing_distance);
  smg(uint32_t, seeing_distance);
  smg(EntityType, type);

  msgpack_object_array const *coords_array = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "coords");
  assert(coords_array->size == 2);

  Point *coords = point_new(coords_array->ptr[0].via.u64, coords_array->ptr[1].via.u64);

  Entity *entity = calloc(1, sizeof(Entity));

#define assign(t) entity->_##t = t

  assign(lp);
  assign(starting_lp);
  assign(mental_health);
  assign(starting_mental_health);
  assign(hunger);
  assign(thirst);
  assign(tiredness);
  assign(xp);
  assign(current_level);
  assign(hearing_distance);
  assign(seeing_distance);
  assign(type);
  assign(coords);

  msgpack_object_str const *name_ptr = serde_map_get(map, MSGPACK_OBJECT_STR, "name");

  entity->_name = calloc(name_ptr->size, sizeof(char));
  memcpy(entity->_name, name_ptr->ptr, name_ptr->size);

  msgpack_object_array const *inventory = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "inventory");

  entity->_inventory = calloc(inventory->size + 1 /* for the null pointer */, sizeof(Item *));
  entity->_inventory[inventory->size] = nullptr;
  for (uint i = 0; i < inventory->size; i++) {
    entity->_inventory[i] = item_deserialize(&(inventory->ptr[i].via.map));
  }

  // TODO: work on perks serial/deserial
  entity->_perks = calloc(1, sizeof(Perk *));
  entity->_perks[0] = nullptr;

  return entity;
}

void entity_serialize(Entity const *ent, msgpack_sbuffer *buffer) {
  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  msgpack_pack_map(&packer, 15);

#define PACK_UINT(t, s)        \
  serde_pack_str(&packer, #t); \
  msgpack_pack_uint##s(&packer, ent->_##t);

  PACK_UINT(lp, 32);
  PACK_UINT(starting_lp, 32);
  PACK_UINT(mental_health, 32);
  PACK_UINT(starting_mental_health, 32);
  PACK_UINT(hunger, 32);
  PACK_UINT(thirst, 32);
  PACK_UINT(tiredness, 32);
  PACK_UINT(xp, 32);
  PACK_UINT(current_level, 32);
  PACK_UINT(hearing_distance, 32);
  PACK_UINT(seeing_distance, 32);
  PACK_UINT(type, 8);

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

void entity_free(Entity *entity) {
  free(entity->_name);

  point_free(entity->_coords);

  entity_inventory_clear(entity);
  free(entity->_inventory);

  entity_perks_clear(entity);
  free(entity->_perks);

  free(entity);
}

inline uint32_t entity_get_life_points(Entity const *entity) {
  return entity->_lp;
}

inline uint32_t entity_get_starting_life_points(Entity const *entity) {
  return entity->_starting_lp;
}

GENERATE_GETTER(uint32_t, mental_health);
GENERATE_GETTER(uint32_t, starting_mental_health);
GENERATE_GETTER(uint32_t, hunger);
GENERATE_GETTER(uint32_t, thirst);
GENERATE_GETTER(uint32_t, tiredness);
GENERATE_GETTER(uint32_t, xp);
GENERATE_GETTER(uint32_t, current_level);
GENERATE_GETTER(uint32_t, hearing_distance);
GENERATE_GETTER(uint32_t, seeing_distance);

inline EntityType entity_get_entity_type(Entity const *entity) {
  return entity->_type;
}

GENERATE_GETTER(char const *, name);
GENERATE_GETTER(Point const *, coords);

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
    case WATER:
      ret = false;
      break;
  }

  return ret && entity_is_alive(ent);
}

inline bool entity_is_alive(Entity const *entity) {
  return entity->_lp > 0;
}

inline bool entity_is_dead(Entity const *entity) {
  return !entity_is_alive(entity);
}

inline bool entity_is_sane(Entity const *entity) {
  return entity->_mental_health > 0;
}

inline bool entity_is_crazy(Entity const *entity) {
  return !entity_is_sane(entity);
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

void entity_mental_hurt(Entity *entity, uint32_t mental_damage) {
  if (mental_damage > entity->_mental_health) {
    entity->_mental_health = 0;
  } else {
    entity->_mental_health = entity->_mental_health - mental_damage;
  }
}

void entity_heal(Entity *entity, uint32_t life_points) {
  if (entity->_lp > 0) {
    entity->_lp = min(entity->_starting_lp, entity->_lp + life_points);
  }
}

void entity_mental_heal(Entity *entity, uint32_t mental_heal) {
  entity->_mental_health += mental_heal;
  if (entity->_mental_health > entity->_starting_mental_health) {
    entity->_mental_health = entity->_starting_mental_health;
  }
}

void entity_resurrect(Entity *entity) {
  if (entity_get_entity_type(entity) == INHUMAN && entity_is_dead(entity)) {
    LOG_INFO("Resurrecting '%s'", entity_get_name(entity));
    entity->_lp = entity->_starting_lp;
  }
}

void entity_increment_hunger(Entity *entity) {
  entity->_hunger++;
}

void entity_increment_thirst(Entity *entity) {
  entity->_thirst++;
}

void entity_increment_tiredness(Entity *entity) {
  entity->_tiredness++;
}

GENERATE_SETTER(hunger);
GENERATE_SETTER(thirst);
GENERATE_SETTER(tiredness);
GENERATE_SETTER(xp);
GENERATE_SETTER(current_level);

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

size_t entity_perks_count(const Entity *self) {
  size_t      count = 0;
  Perk const *ptr = self->_perks[count];

  while (ptr != nullptr) {
    ptr = self->_perks[++count];
  }

  return count;
}

void entity_perks_add(Entity *self, Perk *perk) {
  // Get first valid slot
  size_t first_null = entity_perks_count(self);

  // Need to take nullptr into account
  self->_perks = realloc(self->_perks, (first_null + 2) * sizeof(Perk *));
  self->_perks[first_null] = perk;
  self->_perks[first_null + 1] = nullptr;
}

void entity_perks_remove(Entity *self, const char *perk_name) {
  bool    perk_found = false;
  uint8_t perk_index = 0;
  size_t  perk_count = entity_perks_count(self);

  for (size_t i = 0; i < perk_count; i++) {
    if (strings_equal(perk_get_name(self->_perks[i]), perk_name)) {
      perk_found = true;
      perk_index = i;
    }
  }

  if (perk_found) {
    perk_free(self->_perks[perk_index]);
    self->_perks[perk_index] = self->_perks[perk_count - 1];
    self->_perks[perk_count - 1] = nullptr;
    self->_perks = realloc(self->_perks, perk_count * sizeof(Perk *));
  }
}

bool entity_perks_has_perk(Entity const *self, const char *perk_name) {
  for (size_t i = 0; i < entity_perks_count(self); i++) {
    if (strings_equal(perk_get_name(self->_perks[i]), perk_name)) {
      return true;
    }
  }

  return false;
}

void entity_perks_clear(Entity *self) {
  for (size_t i = 0; i < entity_perks_count(self); i++) {
    perk_free(self->_perks[i]);
  }

  self->_perks = realloc(self->_perks, 1 * sizeof(Perk *));
  self->_perks[0] = nullptr;
}

Perk **entity_perks_get_all(Entity const *self) {
  return self->_perks;
}

Perk **entity_perks_filter(Entity const *self, bool (*filter_fn)(Perk const *), size_t *list_size) {
  size_t items = 0;
  Perk **result = calloc(1, sizeof(Perk *));
  result[0] = nullptr;

  for (size_t i = 0; i < entity_perks_count(self); i++) {
    if (filter_fn(self->_perks[i])) {
      items++;
      result = realloc(result, (items + 1) * sizeof(Perk *));
      result[items - 1] = self->_perks[i];
      result[items] = nullptr;
    }
  }

  *list_size = items;
  return result;
}

Perk const *entity_perks_get(Entity const *self, char const *perk_name) {
  for (size_t i = 0; i < entity_perks_count(self); i++) {
    if (strings_equal(perk_get_name(self->_perks[i]), perk_name)) {
      return self->_perks[i];
    }
  }

  return nullptr;
}
