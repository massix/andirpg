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

#include "tile.h"
#include "item.h"
#include "point.h"
#include "serde.h"
#include <assert.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Tile {
  TileKind _tile_kind;
  uint32_t _base_noise;
  uint32_t _base_light;
  bool     _inside;
  bool     _traversable;
  Item   **_items;
  Point   *_coords;
} Tile;

Tile *tile_new(TileKind kind, uint32_t x, uint32_t y) {
  Tile *tile = calloc(1, sizeof(Tile));

  tile->_tile_kind = kind;
  tile->_base_noise = 3;
  tile->_base_light = 10;
  tile->_inside = false;
  tile->_traversable = true;
  tile->_items = calloc(1, sizeof(Item *));
  tile->_items[0] = nullptr;
  tile->_coords = point_new(x, y);

  return tile;
}

Tile *tile_deserialize(msgpack_object_map const *map) {
  assert(map->size == 7);
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "kind");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "base_noise");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "base_light");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "inside");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "traversable");
  serde_map_assert(map, MSGPACK_OBJECT_ARRAY, "items");
  serde_map_assert(map, MSGPACK_OBJECT_ARRAY, "coords");

  msgpack_object_array const *items = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "items");
  msgpack_object_array const *coords = serde_map_get(map, MSGPACK_OBJECT_ARRAY, "coords");

  assert(coords->size == 2);

  Tile *tile = calloc(1, sizeof(Tile));

  tile->_tile_kind = *(TileKind *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "kind");
  tile->_base_noise = *(uint32_t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "base_noise");
  tile->_base_light = *(uint32_t *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "base_light");
  tile->_inside = *(bool *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "inside");
  tile->_traversable = *(bool *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "traversable");

  tile->_items = calloc(items->size + 1, sizeof(Item *));
  tile->_items[items->size] = nullptr;
  for (uint32_t i = 0; i < items->size; i++) {
    tile->_items[i] = item_deserialize(&items->ptr[i].via.map);
  }

  tile->_coords = point_new(coords->ptr[0].via.u64, coords->ptr[1].via.u64);

  return tile;
}

void tile_serialize(Tile const *tile, msgpack_sbuffer *sbuffer) {
  msgpack_packer *packer = msgpack_packer_new(sbuffer, &msgpack_sbuffer_write);
  msgpack_pack_map(packer, 7);

  serde_pack_str(packer, "kind");
  msgpack_pack_uint8(packer, tile->_tile_kind);

  serde_pack_str(packer, "base_noise");
  msgpack_pack_uint32(packer, tile->_base_noise);

  serde_pack_str(packer, "base_light");
  msgpack_pack_uint32(packer, tile->_base_light);

  serde_pack_str(packer, "inside");
  msgpack_pack_uint8(packer, tile->_inside);

  serde_pack_str(packer, "traversable");
  msgpack_pack_uint8(packer, tile->_traversable);

  serde_pack_str(packer, "items");

  uint32_t items_size = tile_count_items(tile);
  msgpack_pack_array(packer, items_size);
  for (uint32_t i = 0; i < items_size; i++) {
    item_serialize(tile->_items[i], sbuffer);
  }

  serde_pack_str(packer, "coords");
  msgpack_pack_array(packer, 2);
  msgpack_pack_uint32(packer, point_get_x(tile->_coords));
  msgpack_pack_uint32(packer, point_get_y(tile->_coords));

  msgpack_packer_free(packer);
}

void tile_free(Tile *tile) {
  uint32_t index = 0;
  Item    *current_item = tile->_items[index];
  while (current_item != nullptr) {
    item_free(current_item);
    current_item = tile->_items[++index];
  }

  point_free(tile->_coords);
  free(tile);
}

inline TileKind tile_get_tile_kind(Tile const *tile) {
  return tile->_tile_kind;
}

inline bool tile_is_inside(Tile const *tile) {
  return tile->_inside;
}

inline bool tile_is_traversable(Tile const *tile) {
  return tile->_traversable;
}

inline uint32_t tile_get_base_noise(Tile const *tile) {
  return tile->_base_noise;
}

inline uint32_t tile_get_base_light(Tile const *tile) {
  return tile->_base_light;
}

inline Point const *tile_get_coords(Tile const *tile) {
  return tile->_coords;
}

inline uint32_t tile_count_items(Tile const *tile) {
  uint32_t    item_count = 0;
  Item const *current_item = tile->_items[item_count];
  while (current_item != nullptr) {
    current_item = tile->_items[++item_count];
  }

  return item_count;
}

inline void tile_set_base_light(Tile *tile, uint32_t base_light) {
  if (base_light <= 10) {
    tile->_base_light = base_light;
  }
}

inline void tile_set_inside(Tile *tile, bool inside) {
  tile->_inside = inside;
}

inline void tile_set_traversable(Tile *tile, bool traversable) {
  tile->_traversable = traversable;
}

inline void tile_set_kind(Tile *tile, TileKind kind) {
  tile->_tile_kind = kind;
}

