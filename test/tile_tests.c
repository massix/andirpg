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

#include "item.h"
#include "point.h"
#include "tile.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <msgpack/unpack.h>
#include <string.h>

void tile_constructor_test(void) {
  Tile *tile = tile_new(GRASS, 10, 20);
  tile_set_traversable(tile, false);
  tile_set_base_light(tile, 8);

  CU_ASSERT_EQUAL(point_get_x(tile_get_coords(tile)), 10);
  CU_ASSERT_EQUAL(point_get_y(tile_get_coords(tile)), 20);
  CU_ASSERT_EQUAL(tile_get_tile_kind(tile), GRASS);
  CU_ASSERT_EQUAL(tile_get_base_noise(tile), 3);
  CU_ASSERT_EQUAL(tile_get_base_light(tile), 8);
  CU_ASSERT_FALSE(tile_is_traversable(tile));
  CU_ASSERT_FALSE(tile_is_inside(tile));

  tile_free(tile);
}

void tile_serialize_test(void) {
  Tile *tile = tile_new(GRAVIER, 10, 20);
  tile_set_base_light(tile, 3);
  tile_add_item(tile, armor_new("Some Armor", 10, 0, 3, 10, 4));

  msgpack_sbuffer sbuffer;
  msgpack_sbuffer_init(&sbuffer);

  tile_serialize(tile, &sbuffer);
  CU_ASSERT_PTR_NOT_NULL(sbuffer.data);
  CU_ASSERT_FALSE(sbuffer.size == 0);

  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, sbuffer.size);
  memcpy(msgpack_unpacker_buffer(&unpacker), sbuffer.data, sbuffer.size);
  msgpack_unpacker_buffer_consumed(&unpacker, sbuffer.size);

  msgpack_unpacked result;
  msgpack_unpacked_init(&result);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);
  CU_ASSERT_EQUAL(result.data.type, MSGPACK_OBJECT_MAP);
  CU_ASSERT_EQUAL(result.data.via.map.size, 7);

  Tile *rebuilt = tile_deserialize(&result.data.via.map);
  CU_ASSERT_TRUE(points_equal(tile_get_coords(rebuilt), tile_get_coords(tile)));
  CU_ASSERT_EQUAL(tile_get_tile_kind(rebuilt), tile_get_tile_kind(tile));
  CU_ASSERT_EQUAL(tile_get_base_light(rebuilt), tile_get_base_light(tile));
  CU_ASSERT_EQUAL(tile_is_inside(rebuilt), tile_is_inside(tile));
  CU_ASSERT_EQUAL(tile_is_traversable(rebuilt), tile_is_traversable(tile));
  CU_ASSERT_EQUAL(tile_count_items(rebuilt), tile_count_items(tile));

  msgpack_unpacked_destroy(&result);
  msgpack_unpacker_destroy(&unpacker);
  msgpack_sbuffer_destroy(&sbuffer);
  tile_free(rebuilt);
  tile_free(tile);
}

void tile_items_test(void) {
  Tile *tile = tile_new(TALL_GRASS, 0, 0);
  CU_ASSERT_EQUAL(tile_count_items(tile), 0);

  tile_add_item(tile, tool_new("Pickaxe", 10, 3, 2, 15));
  CU_ASSERT_EQUAL(tile_count_items(tile), 1);
  CU_ASSERT_TRUE(strings_equal(item_get_name(tile_get_item_at(tile, 0)), "Pickaxe"));

  tile_add_item(tile, weapon_new("A gun", 10, 30, 1, 50, 10));
  CU_ASSERT_EQUAL(tile_count_items(tile), 2);
  CU_ASSERT_TRUE(strings_equal(item_get_name(tile_get_item_at(tile, 1)), "A gun"));

  tile_add_item(tile, armor_new("Hair", 0, 0, 0, 1, 0));
  CU_ASSERT_EQUAL(tile_count_items(tile), 3);
  CU_ASSERT_TRUE(strings_equal(item_get_name(tile_get_item_at(tile, 2)), "Hair"));

  CU_ASSERT_PTR_NOT_NULL(tile_get_item_with_name(tile, "Hair"));
  CU_ASSERT_PTR_NOT_NULL(tile_get_item_with_name(tile, "A gun"));
  CU_ASSERT_PTR_NOT_NULL(tile_get_item_with_name(tile, "Pickaxe"));
  CU_ASSERT_PTR_NULL(tile_get_item_with_name(tile, "Not existing"));

  tile_remove_item(tile, "Pickaxe");
  CU_ASSERT_EQUAL(tile_count_items(tile), 2);
  CU_ASSERT_TRUE(strings_equal(item_get_name(tile_get_item_at(tile, 0)), "Hair"));

  tile_remove_item(tile, "A gun");
  CU_ASSERT_EQUAL(tile_count_items(tile), 1);
  CU_ASSERT_PTR_NULL(tile_get_item_at(tile, 1));

  tile_remove_item(tile, "Hair");
  CU_ASSERT_EQUAL(tile_count_items(tile), 0);
  CU_ASSERT_PTR_NULL(tile_get_item_at(tile, 0));

  tile_free(tile);
}

void tile_test_suite(void) {
  CU_pSuite suite = CU_add_suite("Tile Tests", nullptr, nullptr);
  CU_add_test(suite, "Constructor and basic functions", &tile_constructor_test);
  CU_add_test(suite, "Serialization and deserialization", &tile_serialize_test);
  CU_add_test(suite, "Items", &tile_items_test);
}
