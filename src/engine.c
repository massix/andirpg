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

#include "engine.h"
#include "entity.h"
#include "logger.h"
#include "map.h"
#include "serde.h"
#include <assert.h>
#include <msgpack.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <point.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct Engine {
  Map     *_map;
  uint32_t _current_cycle;
  Entity  *_active_entity;
};

Engine *engine_new(Map *map) {
  LOG_DEBUG("Creating new engine", 0);
  Engine *ret = calloc(1, sizeof(Engine));
  ret->_map = map;
  ret->_current_cycle = 0;
  ret->_active_entity = nullptr;
  return ret;
}

void engine_serialize(Engine *eng, msgpack_sbuffer *buffer) {
  LOG_DEBUG("Starting packer for engine", eng);

  msgpack_packer packer;

  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);

  // Root object is a map
  msgpack_pack_map(&packer, 3);
  serde_pack_str(&packer, "active_entity");

  if (engine_has_active_entity(eng)) {
    LOG_DEBUG("Engine has active entity", 0);
    serde_pack_str(&packer, entity_get_name(eng->_active_entity));
  } else {
    LOG_WARNING("No active entity found!", 0);
    msgpack_pack_nil(&packer);
  }

  serde_pack_str(&packer, "current_cycle");
  msgpack_pack_unsigned_int(&packer, eng->_current_cycle);

  serde_pack_str(&packer, "map_object");
  map_serialize(eng->_map, buffer);
}

Engine *engine_deserialize(msgpack_object_map const *map) {
  assert(map->size == 3);

  assert(serde_map_find(map, MSGPACK_OBJECT_STR, "active_entity") != nullptr ||
         serde_map_find(map, MSGPACK_OBJECT_NIL, "active_entity") != nullptr);
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "current_cycle");
  serde_map_assert(map, MSGPACK_OBJECT_MAP, "map_object");

  Engine *engine = calloc(1, sizeof(Engine));
  engine->_current_cycle = *(uint32_t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "current_cycle");
  engine->_map = map_deserialize((msgpack_object_map *)serde_map_get(map, MSGPACK_OBJECT_MAP, "map_object"));

  msgpack_object_str const *active_entity = serde_map_get(map, MSGPACK_OBJECT_STR, "active_entity");

  if (active_entity != nullptr) {
    char entity_name[active_entity->size];
    memcpy(entity_name, active_entity->ptr, active_entity->size);
    engine->_active_entity = map_get_entity(engine->_map, entity_name);
  }

  return engine;
}

void engine_free(Engine *engine) {
  map_free(engine->_map);
  free(engine);
}

inline Map *engine_get_map(Engine const *engine) {
  return engine->_map;
}

Entity *engine_get_active_entity(Engine const *engine) {
  Entity *ret = nullptr;
  if (engine_has_active_entity(engine)) {
    ret = engine->_active_entity;
  }

  return ret;
}

inline uint32_t engine_get_current_cycle(Engine const *eng) {
  return eng->_current_cycle;
}

inline bool engine_has_active_entity(Engine const *engine) {
  return engine->_active_entity != nullptr;
}

void engine_set_active_entity(Engine *engine, const char *name) {
  LOG_DEBUG("Setting active entity: '%s'", name);
  if (map_contains_entity(engine->_map, name)) {
    engine->_active_entity = map_get_entity(engine->_map, name);
  } else {
    LOG_WARNING("Engine does not have entity '%s'", name);
    engine->_active_entity = nullptr;
  }
}

inline void engine_clear_active_entity(Engine *engine) {
  engine->_active_entity = nullptr;
}

void engine_move_entity(Engine const *engine, Entity *entity, uint32_t delta_x, uint32_t delta_y) {

  MapBoundaries boundaries = map_get_boundaries(engine->_map);
  Point const  *current = entity_get_coords(entity);
  LOG_DEBUG("Moving entity '%s' (%d, %d)", entity_get_name(entity), delta_x, delta_y);

  if (!map_is_tile_free(engine->_map, point_get_x(current) + delta_x, point_get_y(current) + delta_y)) {
    LOG_WARNING("Entity '%s' wants to move to an occupied tile!", entity_get_name(entity));
    return;
  }

  if (point_get_x(current) + delta_x >= 0 && point_get_x(current) + delta_x < boundaries.x) {
    if (point_get_y(current) + delta_y >= 0 && point_get_y(current) + delta_y < boundaries.y) {
      entity_move(entity, delta_x, delta_y);
    }
  }
}

void engine_move_active_entity(Engine *engine, uint32_t delta_x, uint32_t delta_y) {
  if (engine_has_active_entity(engine)) {
    engine_move_entity(engine, engine->_active_entity, delta_x, delta_y);
  }
}

void engine_handle_keypress(Engine *engine, char key) {
  if (!engine_has_active_entity(engine)) {
    LOG_WARNING("Refusing to handle keypress, no active entity is set!", 0);
    return;
  }

  LOG_DEBUG("Handling key '%c'", key);

  switch (key) {
    // Move the current active entity left
    case 'h':
      engine_move_active_entity(engine, -1, 0);
      break;

    // Move right
    case 'l':
      engine_move_active_entity(engine, 1, 0);
      break;

    // Move up
    case 'k':
      engine_move_active_entity(engine, 0, -1);
      break;

    // Move down
    case 'j':
      engine_move_active_entity(engine, 0, 1);
      break;

    // Diagonal down-left
    case 'b':
      engine_move_active_entity(engine, -1, 1);
      break;

    // Diagonal down-right
    case 'n':
      engine_move_active_entity(engine, 1, 1);
      break;

    // Diagonal up-left
    case 'y':
      engine_move_active_entity(engine, -1, -1);
      break;

    // Diagonal up-right
    case 'u':
      engine_move_active_entity(engine, 1, -1);
      break;

    default:
      break;
  }

  engine->_current_cycle++;
}

bool entities_are_close(Entity const *lhs, Entity const *rhs) {
  Point const *lhs_coords = entity_get_coords(lhs);
  Point const *rhs_coords = entity_get_coords(rhs);

  uint32_t delta_x = point_get_x(lhs_coords) > point_get_x(rhs_coords) ? point_get_x(lhs_coords) - point_get_x(rhs_coords)
                                                                       : point_get_x(rhs_coords) - point_get_x(lhs_coords);
  uint32_t delta_y = point_get_y(lhs_coords) > point_get_y(rhs_coords) ? point_get_y(lhs_coords) - point_get_y(rhs_coords)
                                                                       : point_get_y(rhs_coords) - point_get_y(lhs_coords);

  return delta_y < 2 && delta_x < 2;
}

// Move all entities apart from the active one
void engine_move_all_entities(Engine const *engine) {
  LOG_DEBUG("Moving all entities", 0);
  Entity **all_entities = map_get_all_entities(engine->_map);

  for (uint32_t i = 0; i < map_count_entities(engine->_map); i++) {
    Entity *current_entity = all_entities[i];

    if ((current_entity != engine->_active_entity) && entity_can_move(current_entity)) {
      Point const *current_coords = entity_get_coords(current_entity);

      uint32_t cur_x = point_get_x(current_coords);
      uint32_t cur_y = point_get_y(current_coords);
      uint32_t random_x = (random() % 3) - 1;
      uint32_t random_y = (random() % 3) - 1;
      LOG_INFO("Randomly moving entity '%s' (%d:%d)", entity_get_name(current_entity), random_x, random_y);

      if (map_is_tile_free(engine->_map, cur_x + random_x, cur_y + random_y)) {
        engine_move_entity(engine, current_entity, random_x, random_y);
      } else if (map_is_tile_free(engine->_map, cur_x + random_x, cur_y)) {
        engine_move_entity(engine, current_entity, random_x, 0);
      } else if (map_is_tile_free(engine->_map, cur_x, cur_y + random_y)) {
        engine_move_entity(engine, current_entity, 0, random_y);
      }
    }
  }
}

Entity **engine_get_close_entities(Engine const *engine, ssize_t *size) {
  Entity **ret = nullptr;

  Entity const *active = engine_get_active_entity(engine);
  Entity      **all_entities = map_get_all_entities(engine_get_map(engine));
  *size = 0;

  for (uint32_t i = 0; i < map_count_entities(engine_get_map(engine)); i++) {
    if (all_entities[i] == active) {
      continue;
    }

    if (entities_are_close(active, all_entities[i])) {
      (*size)++;
      ret = realloc(ret, *size * sizeof(Entity *));
      ret[*size - 1] = all_entities[i];
    }
  }

  return ret;
}

inline void engine_add_entity(Engine *engine, Entity *ent) {
  map_add_entity(engine->_map, ent);
}

void engine_entity_attack(Engine *engine, Entity *lhs, Entity *rhs) {
  LOG_DEBUG("Entity '%s' wants to attack '%s'", entity_get_name(lhs), entity_get_name(rhs));
  if (entities_are_close(lhs, rhs)) {
    LOG_DEBUG("Entities are close, attack is successful", 0);
    entity_hurt(rhs, 1);
  }
}

