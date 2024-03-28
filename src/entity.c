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
#include "collections/linked_list.h"
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

typedef struct Equipment {
  Item *_head;
  Item *_neck;
  Item *_torso;
  Item *_left_hand;
  Item *_right_hand;
  Item *_legs;
  Item *_left_foot;
  Item *_right_foot;
} Equipment;

struct Entity {
  uint32_t    _lp;
  uint32_t    _starting_lp;
  uint32_t    _mental_health;
  uint32_t    _starting_mental_health;
  uint32_t    _strength;
  uint32_t    _hunger;
  uint32_t    _thirst;
  uint32_t    _tiredness;
  uint32_t    _xp;
  uint32_t    _current_level;
  uint32_t    _hearing_distance;
  uint32_t    _seeing_distance;
  EntityType  _type;
  char       *_name;
  Point      *_coords;
  Item      **_inventory;
  LinkedList *_perks;
  Equipment  *_equipment;
};

Equipment *equipment_new() {
  Equipment *self = calloc(1, sizeof(Equipment));
  self->_head = nullptr;
  self->_neck = nullptr;
  self->_torso = nullptr;
  self->_left_hand = nullptr;
  self->_right_hand = nullptr;
  self->_legs = nullptr;
  self->_left_foot = nullptr;
  self->_right_foot = nullptr;

  return self;
}

void equipment_free(Equipment *self) {
#define free_nonnull(p) \
  if ((p) != nullptr)   \
    item_free(p);

  free_nonnull(self->_head);
  free_nonnull(self->_neck);
  free_nonnull(self->_torso);
  free_nonnull(self->_left_hand);
  free_nonnull(self->_right_hand);
  free_nonnull(self->_legs);
  free_nonnull(self->_left_foot);
  free_nonnull(self->_right_foot);
}

void equipment_serialize(Equipment const *self, msgpack_sbuffer *buffer) {
  msgpack_packer *packer = msgpack_packer_new(buffer, &msgpack_sbuffer_write);
  msgpack_pack_map(packer, 8);

#define serialize_part(part)               \
  serde_pack_str(packer, #part);           \
  if (self->_##part != nullptr)            \
    item_serialize(self->_##part, buffer); \
  else                                     \
    msgpack_pack_nil(packer);

  serialize_part(head);
  serialize_part(neck);
  serialize_part(torso);
  serialize_part(left_hand);
  serialize_part(right_hand);
  serialize_part(legs);
  serialize_part(left_foot);
  serialize_part(right_foot);

  msgpack_packer_free(packer);
}

Equipment *equipment_deserialize(msgpack_object_map const *map) {
  assert(map->size == 8);

  Equipment *self = equipment_new();

#define deserialize_part(part)                                       \
  msgpack_object_kv const *kv_##part = serde_map_find_l(map, #part); \
  if (kv_##part->val.type == MSGPACK_OBJECT_MAP) {                   \
    self->_##part = item_deserialize(&kv_##part->val.via.map);       \
  }

  deserialize_part(head);
  deserialize_part(neck);
  deserialize_part(torso);
  deserialize_part(left_hand);
  deserialize_part(right_hand);
  deserialize_part(legs);
  deserialize_part(left_foot);
  deserialize_part(right_foot);

  return self;
}

#define EQUIPMENT_GETTER(part)                        \
  Item *equipment_get_##part(Equipment const *self) { \
    return self->_##part;                             \
  }

#define EQUIPMENT_SETTER(part)                             \
  void equipment_set_##part(Equipment *self, Item *item) { \
    self->_##part = item;                                  \
  }

#define EQUIPMENT_CLEARER(part)                  \
  void equipment_clear_##part(Equipment *self) { \
    if (self->_##part != nullptr) {              \
      item_free(self->_##part);                  \
      self->_##part = nullptr;                   \
    }                                            \
  }

EQUIPMENT_GETTER(head);
EQUIPMENT_SETTER(head);
EQUIPMENT_CLEARER(head);
EQUIPMENT_GETTER(neck);
EQUIPMENT_SETTER(neck);
EQUIPMENT_CLEARER(neck);
EQUIPMENT_GETTER(torso);
EQUIPMENT_SETTER(torso);
EQUIPMENT_CLEARER(torso);
EQUIPMENT_GETTER(left_hand);
EQUIPMENT_SETTER(left_hand);
EQUIPMENT_CLEARER(left_hand);
EQUIPMENT_GETTER(right_hand);
EQUIPMENT_SETTER(right_hand);
EQUIPMENT_CLEARER(right_hand);
EQUIPMENT_GETTER(legs);
EQUIPMENT_SETTER(legs);
EQUIPMENT_CLEARER(legs);
EQUIPMENT_GETTER(left_foot);
EQUIPMENT_SETTER(left_foot);
EQUIPMENT_CLEARER(left_foot);
EQUIPMENT_GETTER(right_foot);
EQUIPMENT_SETTER(right_foot);
EQUIPMENT_CLEARER(right_foot);

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

EntityBuilder *eb_with_strength(EntityBuilder *self, uint32_t strength) {
  self->strength = strength;
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
  ent->_strength = self->strength;
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
  ent->_perks = linked_list_new(32, (FreeFunction)&perk_free);
  ent->_equipment = equipment_new();

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
  builder->strength = 10;
  builder->x = 0;
  builder->y = 0;
  builder->hunger = 0;
  builder->thirst = 0;
  builder->tiredness = 0;

  // Methods
  builder->with_type = &eb_with_type;
  builder->with_life_points = &eb_with_lp;
  builder->with_mental_health = &eb_with_mh;
  builder->with_strength = &eb_with_strength;
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

  assert(map->size == 18);

#define sma(t, f) serde_map_assert(map, MSGPACK_OBJECT_##t, f);

  sma(POSITIVE_INTEGER, "lp");
  sma(POSITIVE_INTEGER, "starting_lp");
  sma(POSITIVE_INTEGER, "mental_health");
  sma(POSITIVE_INTEGER, "starting_mental_health");
  sma(POSITIVE_INTEGER, "strength");
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
  sma(MAP, "equipment");
  sma(ARRAY, "inventory");
  sma(ARRAY, "perks");

#define smg(t, f) t f = *(t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, #f);

  // NOLINTNEXTLINE(readability-identifier-length)
  smg(uint32_t, lp);
  smg(uint32_t, starting_lp);
  smg(uint32_t, mental_health);
  smg(uint32_t, starting_mental_health);
  smg(uint32_t, hunger);
  smg(uint32_t, thirst);
  smg(uint32_t, tiredness);
  smg(uint32_t, strength);
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
  assign(strength);
  assign(xp);
  assign(current_level);
  assign(hearing_distance);
  assign(seeing_distance);
  assign(type);
  assign(coords);

  msgpack_object_str const *name_ptr = serde_map_get(map, MSGPACK_OBJECT_STR, "name");

  entity->_name = calloc(name_ptr->size, sizeof(char));
  memcpy(entity->_name, name_ptr->ptr, name_ptr->size);

  msgpack_object_map const *equipment = serde_map_get(map, MSGPACK_OBJECT_MAP, "equipment");
  entity->_equipment = equipment_deserialize(equipment);

  msgpack_object_array const *inventory = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "inventory");

  entity->_inventory = calloc(inventory->size + 1 /* for the null pointer */, sizeof(Item *));
  entity->_inventory[inventory->size] = nullptr;
  for (uint i = 0; i < inventory->size; i++) {
    entity->_inventory[i] = item_deserialize(&(inventory->ptr[i].via.map));
  }

  msgpack_object_array const *perks = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "perks");
  entity->_perks = linked_list_new(32, (FreeFunction)&perk_free);
  for (uint i = 0; i < perks->size; i++) {
    linked_list_add(entity->_perks, perk_deserialize(&(perks->ptr[i].via.map)));
  }

  return entity;
}

void entity_serialize(Entity const *ent, msgpack_sbuffer *buffer) {
  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  msgpack_pack_map(&packer, 18);

#define PACK_UINT(t, s)        \
  serde_pack_str(&packer, #t); \
  msgpack_pack_uint##s(&packer, ent->_##t);

  PACK_UINT(lp, 32);
  PACK_UINT(starting_lp, 32);
  PACK_UINT(mental_health, 32);
  PACK_UINT(starting_mental_health, 32);
  PACK_UINT(strength, 32);
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

  serde_pack_str(&packer, "equipment");
  equipment_serialize(ent->_equipment, buffer);

  serde_pack_str(&packer, "inventory");
  uint32_t inventory_count = entity_inventory_count(ent);
  msgpack_pack_array(&packer, inventory_count);
  for (uint32_t i = 0; i < inventory_count; i++) {
    item_serialize(ent->_inventory[i], buffer);
  }

  serde_pack_str(&packer, "perks");
  msgpack_pack_array(&packer, linked_list_count(ent->_perks));
  linked_list_iterator_reset(ent->_perks);
  for (Perk const *current = linked_list_iterator_next(ent->_perks); current != nullptr; current = linked_list_iterator_next(ent->_perks)) {
    perk_serialize(current, buffer);
  }
}

void entity_free(Entity *entity) {
  free(entity->_name);

  point_free(entity->_coords);

  entity_inventory_clear(entity);
  free(entity->_inventory);

  linked_list_free(entity->_perks);
  equipment_free(entity->_equipment);

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
GENERATE_GETTER(uint32_t, strength);
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

// Private method
Item *entity_inventory_pop(Entity *self, char const *name, ItemType type) {
  uint  index = 0;
  Item *new_item = nullptr;

  Item const *iterator = self->_inventory[index];
  while (iterator != nullptr) {
    if (strings_equal(item_get_name(iterator), name) && item_get_type(iterator) == type) {
      break;
    }

    iterator = self->_inventory[++index];
  }

  if (iterator != nullptr) {
    new_item = item_clone(iterator);
    entity_inventory_remove_item(self, name);
  }

  return new_item;
}

Item *entity_equipment_get_head(Entity const *self) {
  return equipment_get_head(self->_equipment);
}

void entity_equipment_set_head(Entity *self, char const *item) {
  if (entity_equipment_get_head(self) != nullptr) {
    return;
  }

  Item *head_gear = entity_inventory_pop(self, item, ARMOR);

  if (head_gear != nullptr) {
    equipment_set_head(self->_equipment, head_gear);
  }
}

void entity_equipment_unset_head(Entity *self) {
  Item const *head_gear = equipment_get_head(self->_equipment);
  if (head_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(head_gear));
    equipment_clear_head(self->_equipment);
  }
}

Item *entity_equipment_get_neck(Entity const *self) {
  return equipment_get_neck(self->_equipment);
}

void entity_equipment_set_neck(Entity *self, char const *item) {
  if (entity_equipment_get_neck(self) != nullptr) {
    return;
  }

  Item *neck_gear = entity_inventory_pop(self, item, ARMOR);

  if (neck_gear != nullptr) {
    equipment_set_neck(self->_equipment, neck_gear);
  }
}

void entity_equipment_unset_neck(Entity *self) {
  Item const *neck_gear = equipment_get_neck(self->_equipment);
  if (neck_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(neck_gear));
    equipment_clear_neck(self->_equipment);
  }
}

Item *entity_equipment_get_torso(Entity const *self) {
  return equipment_get_torso(self->_equipment);
}

void entity_equipment_set_torso(Entity *self, char const *item) {
  if (entity_equipment_get_torso(self) != nullptr) {
    return;
  }

  Item *torso_gear = entity_inventory_pop(self, item, ARMOR);

  if (torso_gear != nullptr) {
    equipment_set_torso(self->_equipment, torso_gear);
  }
}

void entity_equipment_unset_torso(Entity *self) {
  Item const *torso_gear = equipment_get_torso(self->_equipment);
  if (torso_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(torso_gear));
    equipment_clear_torso(self->_equipment);
  }
}

Item *entity_equipment_get_legs(Entity const *self) {
  return equipment_get_legs(self->_equipment);
}

void entity_equipment_set_legs(Entity *self, char const *item) {
  if (entity_equipment_get_legs(self) != nullptr) {
    return;
  }

  Item *legs_gear = entity_inventory_pop(self, item, ARMOR);

  if (legs_gear != nullptr) {
    equipment_set_legs(self->_equipment, legs_gear);
  }
}

void entity_equipment_unset_legs(Entity *self) {
  Item const *legs_gear = equipment_get_legs(self->_equipment);
  if (legs_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(legs_gear));
    equipment_clear_legs(self->_equipment);
  }
}

Item *entity_equipment_get_left_foot(Entity const *self) {
  return equipment_get_left_foot(self->_equipment);
}

void entity_equipment_set_left_foot(Entity *self, char const *item) {
  if (entity_equipment_get_left_foot(self) != nullptr) {
    return;
  }

  Item *left_foot_gear = entity_inventory_pop(self, item, ARMOR);

  if (left_foot_gear != nullptr) {
    equipment_set_left_foot(self->_equipment, left_foot_gear);
  }
}

void entity_equipment_unset_left_foot(Entity *self) {
  Item const *left_foot_gear = equipment_get_left_foot(self->_equipment);
  if (left_foot_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(left_foot_gear));
    equipment_clear_left_foot(self->_equipment);
  }
}

Item *entity_equipment_get_right_foot(Entity const *self) {
  return equipment_get_right_foot(self->_equipment);
}

void entity_equipment_set_right_foot(Entity *self, char const *item) {
  if (entity_equipment_get_right_foot(self) != nullptr) {
    return;
  }

  Item *right_foot_gear = entity_inventory_pop(self, item, ARMOR);

  if (right_foot_gear != nullptr) {
    equipment_set_right_foot(self->_equipment, right_foot_gear);
  }
}

void entity_equipment_unset_right_foot(Entity *self) {
  Item const *right_foot_gear = equipment_get_right_foot(self->_equipment);
  if (right_foot_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(right_foot_gear));
    equipment_clear_right_foot(self->_equipment);
  }
}

Item *entity_equipment_get_right_hand(Entity const *self) {
  return equipment_get_right_hand(self->_equipment);
}

void entity_equipment_set_right_hand(Entity *self, char const *item) {
  // Must check if left hand is two-handed weapon first
  Item const *left_hand_gear = equipment_get_left_hand(self->_equipment);
  if (left_hand_gear != nullptr && weapon_get_hands(item_get_properties(left_hand_gear)) == 2) {
    LOG_WARNING("Already equipping a two-handed weapon in left hand, cannot equip right hand", 0);
    return;
  }

  if (entity_equipment_get_right_hand(self) != nullptr) {
    LOG_WARNING("Cannot equip '%s' as right hand already equipped", item);
    return;
  }

  Item *right_hand_gear = entity_inventory_pop(self, item, WEAPON);

  if (right_hand_gear != nullptr) {
    if (weapon_get_hands(item_get_properties(right_hand_gear)) == 2) {
      LOG_WARNING("Trying to set item '%s' which is two-handed in right hand", item);
      entity_inventory_add_item(self, right_hand_gear);
      entity_equipment_set_left_hand(self, item);
    } else {
      equipment_set_right_hand(self->_equipment, right_hand_gear);
    }
  }
}

void entity_equipment_unset_right_hand(Entity *self) {
  Item const *right_hand_gear = equipment_get_right_hand(self->_equipment);
  if (right_hand_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(right_hand_gear));
    equipment_clear_right_hand(self->_equipment);
  }
}

Item *entity_equipment_get_left_hand(Entity const *self) {
  return equipment_get_left_hand(self->_equipment);
}

void entity_equipment_set_left_hand(Entity *self, char const *item) {
  if (entity_equipment_get_left_hand(self) != nullptr) {
    return;
  }

  Item *left_hand_gear = entity_inventory_pop(self, item, WEAPON);

  if (left_hand_gear != nullptr) {
    equipment_set_left_hand(self->_equipment, left_hand_gear);
  }
}

void entity_equipment_unset_left_hand(Entity *self) {
  Item const *left_hand_gear = equipment_get_left_hand(self->_equipment);
  if (left_hand_gear != nullptr) {
    entity_inventory_add_item(self, item_clone(left_hand_gear));
    equipment_clear_left_hand(self->_equipment);
  }
}

size_t entity_perks_count(const Entity *self) {
  return linked_list_count(self->_perks);
}

void entity_perks_add(Entity *self, Perk *perk) {
  linked_list_add(self->_perks, perk);
}

void entity_perks_remove(Entity *self, const char *perk_name) {
  if (linked_list_is_empty(self->_perks)) {
    return;
  }

  bool    perk_found = false;
  uint8_t perk_index = 0;

  linked_list_iterator_reset(self->_perks);

  for (Perk const *current = linked_list_iterator_next(self->_perks); current != nullptr;
       current = linked_list_iterator_next(self->_perks)) {
    if (strings_equal(perk_get_name(current), perk_name)) {
      perk_found = true;
      break;
    }

    perk_index++;
  }

  if (perk_found) {
    linked_list_remove(self->_perks, perk_index);
  }
}

bool entity_perks_has_perk(Entity const *self, const char *perk_name) {
  linked_list_iterator_reset(self->_perks);
  for (Perk const *current = linked_list_iterator_next(self->_perks); current != nullptr;
       current = linked_list_iterator_next(self->_perks)) {
    if (strings_equal(perk_get_name(current), perk_name)) {
      return true;
    }
  };

  return false;
}

void entity_perks_clear(Entity *self) {
  while (!linked_list_is_empty(self->_perks)) {
    linked_list_remove(self->_perks, 0);
  }

  linked_list_memory_shrink(self->_perks);
}

Perk **entity_perks_filter(Entity const *self, bool (*filter_fn)(Perk const *), size_t *list_size) {
  return (Perk **)linked_list_find_all(self->_perks, (Comparator)filter_fn, list_size);
}

Perk const *entity_perks_get(Entity const *self, char const *perk_name) {
  linked_list_iterator_reset(self->_perks);
  for (Perk const *current = linked_list_iterator_next(self->_perks); current != nullptr;
       current = linked_list_iterator_next(self->_perks)) {
    if (strings_equal(perk_get_name(current), perk_name)) {
      return current;
    }
  }

  return nullptr;
}
