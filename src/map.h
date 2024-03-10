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

#ifndef __MAP__H__
#define __MAP__H__

#include "entity.h"
#include "item.h"
#include "tile.h"
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <ncurses.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Map Map;
typedef struct MapBoundaries {
  uint32_t x;
  uint32_t y;
} MapBoundaries;

// Constructors
Map *map_new(uint32_t x_size, uint32_t y_size, uint32_t max_entities, char const *);
Map *map_deserialize(msgpack_object_map *);

void          map_add_entity(Map *, Entity *);
void          map_add_item(Map *, Item *, uint32_t x, uint32_t y);
void          map_remove_item(Map *, const char *);
bool          map_contains_item(Map *, const char *);
uint32_t      map_count_items(Map *);
char const   *map_get_name(Map const *);
Item         *map_get_item(Map *, const char *);
Entity       *map_get_entity(Map *, const char *);
Entity      **map_get_all_entities(Map *);
Entity      **map_filter_entities(Map *, bool (*)(Entity *), ssize_t *);
void          map_remove_entity(Map *, const char *);
int           map_get_index_of_entity(Map *, const char *);
bool          map_contains_entity(Map *, const char *);
int           map_count_entities(Map *);
MapBoundaries map_get_boundaries(Map *);
bool          map_is_tile_free(Map *, uint32_t x, uint32_t y);
void          map_serialize(Map *, msgpack_sbuffer *);
Tile const   *map_get_tile(Map const *, uint32_t x, uint32_t y);

// Destructor
void map_free(Map *);

#endif
