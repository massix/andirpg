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
} EntityType;

// Constructors and destructors
Entity *entity_new(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y);
Entity *entity_deserialize(msgpack_object_map const *);
void    entity_free(Entity *);
void    entity_serialize(Entity const *, msgpack_sbuffer *);

// Getters
uint32_t     entity_get_life_points(Entity const *);
uint32_t     entity_get_starting_life_points(Entity const *);
EntityType   entity_get_entity_type(Entity const *);
const char  *entity_get_name(Entity const *);
Point const *entity_get_coords(Entity const *);

// Methods
bool entity_can_move(Entity const *);
bool entity_is_alive(Entity const *);
bool entity_is_dead(Entity const *);
void entity_move(Entity *, uint32_t delta_x, uint32_t delta_y);
void entity_hurt(Entity *, uint32_t life_points);
void entity_heal(Entity *, uint32_t life_points);
void entity_resurrect(Entity *);

// Inventory methods
size_t entity_inventory_count(Entity const *);
void   entity_inventory_add_item(Entity *, Item *);
void   entity_inventory_remove_item(Entity *, const char *);
void   entity_inventory_clear(Entity *);
Item **entity_inventory_filter(Entity *, bool (*)(Item const *), ssize_t *);
Item **entity_inventory_get(Entity const *); // PERF: Only useful for tests

#endif
