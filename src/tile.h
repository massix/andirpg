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

#ifndef __TILE__H__
#define __TILE__H__

#include "point.h"
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Tile Tile;
typedef enum TileKind { GRASS, TALL_GRASS, ROAD, GRAVIER, FLOOR } TileKind;

// Constructors and destructors
Tile *tile_new(uint32_t x, uint32_t y);
Tile *tile_deserialize(msgpack_object_map const *);
void  tile_serialize(Tile const *, msgpack_sbuffer *);
void  tile_free(Tile *tile);

// Getters
TileKind     tile_get_tile_kind(Tile const *);
bool         tile_is_inside(Tile const *);
bool         tile_is_traversable(Tile const *);
uint32_t     tile_get_base_noise(Tile const *);
uint32_t     tile_get_base_light(Tile const *);
Point const *tile_get_coords(Tile const *);
uint32_t     tile_count_items(Tile const *);

// Setters
void tile_set_base_light(Tile *, uint32_t);
void tile_set_inside(Tile *, bool);
void tile_set_traversable(Tile *, bool);

#endif /* ifndef __TILE__H__ */

