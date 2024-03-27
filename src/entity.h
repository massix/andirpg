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

#ifndef __ENTITY__H__
#define __ENTITY__H__

#include "item.h"
#include "perk.h"
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <point.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Entity Entity;

typedef enum EntityType {
  HUMAN = '@',
  ANIMAL = 'a',
  INHUMAN = '&',
  TREE = '%',
  MOUNTAIN = '^',
  WATER = '~',
} EntityType;

typedef struct EntityBuilder {
  EntityType type;             // Default: INHUMAN
  uint32_t   life_points;      // Default: 30
  uint32_t   mental_health;    // Default: 30
  uint32_t   level;            // Default: 0
  uint32_t   xp;               // Default: 0
  uint32_t   hearing_distance; // Default: 10
  uint32_t   seeing_distance;  // Default: 10
  uint32_t   x;                // Default: 0
  uint32_t   y;                // Default: 0
  uint32_t   hunger;           // Default: 0
  uint32_t   thirst;           // Default: 0
  uint32_t   tiredness;        // Default: 0
  char      *name;             // Default: nullptr (MANDATORY)

  struct EntityBuilder *(*with_type)(struct EntityBuilder *, EntityType);
  struct EntityBuilder *(*with_life_points)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_mental_health)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_level)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_xp)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_hearing_distance)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_seeing_distance)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_name)(struct EntityBuilder *, char const *);
  struct EntityBuilder *(*with_coords)(struct EntityBuilder *, uint32_t x, uint32_t y);
  struct EntityBuilder *(*with_hunger)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_thirst)(struct EntityBuilder *, uint32_t);
  struct EntityBuilder *(*with_tiredness)(struct EntityBuilder *, uint32_t);
  Entity *(*build)(struct EntityBuilder *, bool oneshot);
} EntityBuilder;

EntityBuilder *entity_builder_new();
void           entity_builder_free(EntityBuilder *);

// Constructors and destructors
Entity *entity_new(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y)
  __attribute__((deprecated("Deprecated constructor, use entity_build() with the same parameters")));
// Commodity constructor based on builder with all the default values
Entity *entity_build(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y);
Entity *entity_deserialize(msgpack_object_map const *);
void    entity_serialize(Entity const *, msgpack_sbuffer *);
void    entity_free(Entity *);

// Getters
uint32_t     entity_get_life_points(Entity const *);
uint32_t     entity_get_starting_life_points(Entity const *);
uint32_t     entity_get_mental_health(Entity const *);
uint32_t     entity_get_starting_mental_health(Entity const *);
uint32_t     entity_get_hunger(Entity const *);
uint32_t     entity_get_thirst(Entity const *);
uint32_t     entity_get_tiredness(Entity const *);
uint32_t     entity_get_xp(Entity const *);
uint32_t     entity_get_current_level(Entity const *);
uint32_t     entity_get_hearing_distance(Entity const *);
uint32_t     entity_get_seeing_distance(Entity const *);
EntityType   entity_get_entity_type(Entity const *);
const char  *entity_get_name(Entity const *);
Point const *entity_get_coords(Entity const *);

// Methods
bool entity_can_move(Entity const *);
bool entity_is_alive(Entity const *);
bool entity_is_dead(Entity const *);
bool entity_is_sane(Entity const *);
bool entity_is_crazy(Entity const *);
void entity_move(Entity *, uint32_t delta_x, uint32_t delta_y);
void entity_hurt(Entity *, uint32_t);
void entity_mental_hurt(Entity *, uint32_t);
void entity_heal(Entity *, uint32_t);
void entity_mental_heal(Entity *, uint32_t);
void entity_resurrect(Entity *);
void entity_increment_hunger(Entity *);
void entity_increment_thirst(Entity *);
void entity_increment_tiredness(Entity *);

// Setters
void entity_set_hunger(Entity *, uint32_t);
void entity_set_thirst(Entity *, uint32_t);
void entity_set_tiredness(Entity *, uint32_t);
void entity_set_xp(Entity *, uint32_t);
void entity_set_current_level(Entity *, uint32_t);

// Inventory methods
size_t entity_inventory_count(Entity const *);
void   entity_inventory_add_item(Entity *, Item *);
void   entity_inventory_remove_item(Entity *, const char *);
void   entity_inventory_clear(Entity *);
Item **entity_inventory_filter(Entity *, bool (*)(Item const *), ssize_t *);
Item **entity_inventory_get(Entity const *); // PERF: Only useful for tests

// Equipment methods
Item *entity_equipment_get_head(Entity const *self);
void  entity_equipment_set_head(Entity *self, char const *item);
void  entity_equipment_unset_head(Entity *self);
Item *entity_equipment_get_neck(Entity const *self);
void  entity_equipment_set_neck(Entity *self, char const *item);
void  entity_equipment_unset_neck(Entity *self);
Item *entity_equipment_get_torso(Entity const *self);
void  entity_equipment_set_torso(Entity *self, char const *item);
void  entity_equipment_unset_torso(Entity *self);
Item *entity_equipment_get_legs(Entity const *self);
void  entity_equipment_set_legs(Entity *self, char const *item);
void  entity_equipment_unset_legs(Entity *self);
Item *entity_equipment_get_left_foot(Entity const *self);
void  entity_equipment_set_left_foot(Entity *self, char const *item);
void  entity_equipment_unset_left_foot(Entity *self);
Item *entity_equipment_get_right_foot(Entity const *self);
void  entity_equipment_set_right_foot(Entity *self, char const *item);
void  entity_equipment_unset_right_foot(Entity *self);
Item *entity_equipment_get_left_hand(Entity const *self);
void  entity_equipment_set_left_hand(Entity *self, char const *item);
void  entity_equipment_unset_left_hand(Entity *self);
Item *entity_equipment_get_right_hand(Entity const *self);
void  entity_equipment_set_right_hand(Entity *self, char const *item);
void  entity_equipment_unset_right_hand(Entity *self);

// Perks methods
size_t      entity_perks_count(Entity const *);
void        entity_perks_add(Entity *, Perk *);
void        entity_perks_remove(Entity *, char const *);
bool        entity_perks_has_perk(Entity const *, char const *);
void        entity_perks_clear(Entity *);
Perk      **entity_perks_filter(Entity const *, bool (*)(Perk const *), size_t *);
Perk const *entity_perks_get(Entity const *, char const *);

#endif
