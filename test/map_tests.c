#include "item.h"
#include "map.h"
#include "point.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <asm-generic/fcntl.h>
#include <entity.h>
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <msgpack/unpack.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void map_creation_test(void) {
  Map *map = map_new(20, 20, 15);
  CU_ASSERT_PTR_NOT_NULL(map);

  MapBoundaries boundaries = map_get_boundaries(map);
  CU_ASSERT_EQUAL(boundaries.x, 20);
  CU_ASSERT_EQUAL(boundaries.y, 20);

  map_free(map);
}

bool filter_zombies(Entity *entity) {
  return *(entity_get_entity_type(entity)) == INHUMAN;
}

void map_entities_test(void) {
  Map *map = map_new(20, 20, 15);
  map_add_entity(map, entity_new(30, INHUMAN, "e1", 10, 10));
  map_add_entity(map, entity_new(15, ANIMAL, "e2", 10, 12));
  map_add_entity(map, entity_new(15, HUMAN, "e3", 9, 12));
  map_add_entity(map, entity_new(20, INHUMAN, "e4", 11, 11));

  Entity    **all = map_get_all_entities(map);
  const char *name = entity_get_name(all[0]);
  CU_ASSERT_TRUE(strings_equal(name, "e1"));

  // This should fail as tile is already occupied
  map_add_entity(map, entity_new(10, INHUMAN, "e5", 11, 11));

  CU_ASSERT_EQUAL(map_count_entities(map), 4);

  CU_ASSERT_TRUE(map_contains_entity(map, "e1"));
  CU_ASSERT_TRUE(map_contains_entity(map, "e2"));
  CU_ASSERT_TRUE(map_contains_entity(map, "e3"));
  CU_ASSERT_TRUE(map_contains_entity(map, "e4"));
  CU_ASSERT_FALSE(map_contains_entity(map, "notexisting"));

  Entity *existing = map_get_entity(map, "e1");
  CU_ASSERT_PTR_NOT_NULL(existing);
  CU_ASSERT_EQUAL(entity_get_life_points(existing), 30);

  int human_index = map_get_index_of_entity(map, "e3");
  CU_ASSERT_EQUAL(human_index, 2);

  Entity *not_existing = map_get_entity(map, "nonexisting");
  CU_ASSERT_PTR_NULL(not_existing);
  CU_ASSERT_FALSE(map_contains_entity(map, "nonexisting"));

  CU_ASSERT_FALSE(map_is_tile_free(map, 10, 12));

  map_remove_entity(map, "e2");
  CU_ASSERT_FALSE(map_contains_entity(map, "e2"));
  CU_ASSERT_EQUAL(map_get_index_of_entity(map, "e3"), 1);
  CU_ASSERT_EQUAL(map_get_index_of_entity(map, "e4"), 2);

  CU_ASSERT_TRUE(map_is_tile_free(map, 10, 12));
  CU_ASSERT_EQUAL(map_count_entities(map), 3);

  // Now I can reuse the tile
  map_add_entity(map, entity_new(10, HUMAN, "e5", 10, 12));
  CU_ASSERT_EQUAL(map_count_entities(map), 4);

  ssize_t  nb_results;
  Entity **filtered = map_filter_entities(map, &filter_zombies, &nb_results);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(filtered[0]), "e1"));
  CU_ASSERT_TRUE(strings_equal(entity_get_name(filtered[1]), "e4"));
  CU_ASSERT_EQUAL(nb_results, 2);

  free(filtered);

  map_free(map);
}

void map_items_test(void) {
  Map *map = map_new(20, 20, 20);
  CU_ASSERT_FALSE(map_contains_item(map, "Non existing"));

  map_add_item(map, weapon_new("Some weapon", 20, 10, 1, 10, 30), 3, 2);
  CU_ASSERT_TRUE(map_contains_item(map, "Some weapon"));

  Item *weapon = map_get_item(map, "Some weapon");
  CU_ASSERT_PTR_NOT_NULL(weapon);
  CU_ASSERT_EQUAL(point_get_x(item_get_coords(weapon)), 3);
  CU_ASSERT_EQUAL(point_get_y(item_get_coords(weapon)), 2);

  CU_ASSERT_PTR_NULL(map_get_item(map, "Not existing"));

  map_add_item(map, tool_new("A tool", 30, 10, 2, 10), 4, 11);
  map_add_item(map, armor_new("An armor", 30, 10, 15, 4, 4), 3, 10);

  CU_ASSERT_EQUAL(map_count_items(map), 3);
  CU_ASSERT_PTR_NOT_NULL(map_get_item(map, "A tool"));

  map_remove_item(map, "A tool");
  CU_ASSERT_EQUAL(map_count_items(map), 2);
  CU_ASSERT_PTR_NULL(map_get_item(map, "A tool"));
  CU_ASSERT_PTR_NOT_NULL(map_get_item(map, "An armor"));

  map_free(map);
}

void check_msgpack_key(msgpack_object *obj, const char *key) {
  size_t str_size = obj->via.str.size;
  char  *str = (char *)malloc(str_size);
  memcpy(str, obj->via.str.ptr, str_size);
  CU_ASSERT_TRUE(strings_equal(str, key));
  free(str);
}

Map *create_serde_map() {
  Map *map = map_new(42, 23, 15);
  map_add_entity(map, entity_new(30, HUMAN, "E1", 12, 0));
  map_add_entity(map, entity_new(15, INHUMAN, "E2", 13, 1));

  // I don't care about the items themselves
  map_add_item(map, armor_new("An armor", 30, 15, 16, 3, 3), 10, 1);
  map_add_item(map, armor_new("A tee-shirt", 30, 15, 16, 3, 3), 10, 1);
  map_add_item(map, armor_new("A blue-jeans", 30, 15, 16, 3, 3), 10, 1);
  map_add_item(map, armor_new("A hat", 30, 15, 16, 3, 3), 10, 1);

  return map;
}

void map_serialization_test() {
  const char *filename = "map_serialization_test.bin";

  Map *map = create_serde_map();

  msgpack_sbuffer sbuffer;
  msgpack_sbuffer_init(&sbuffer);

  map_serialize(map, &sbuffer);
  CU_ASSERT_PTR_NOT_NULL(sbuffer.data);
  CU_ASSERT_TRUE(sbuffer.size > 0);

  // Write to file
  int msgpack_file = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  write(msgpack_file, sbuffer.data, sbuffer.size);
  close(msgpack_file);

  // Read file to a new buffer
  FILE  *input_file = fopen(filename, "rb");
  size_t input_file_length = file_size(input_file);

  char *buffer = (char *)malloc(input_file_length);
  fread(buffer, sizeof(char), input_file_length, input_file);
  fclose(input_file);

  unlink(filename);

  // This assertion *does not guarantee* that the buffers are really the same,
  // but it's a good guard against other issues
  CU_ASSERT_TRUE(strings_equal(sbuffer.data, buffer));

  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, 0);
  msgpack_unpacker_reserve_buffer(&unpacker, input_file_length);
  memcpy(msgpack_unpacker_buffer(&unpacker), buffer, input_file_length);
  msgpack_unpacker_buffer_consumed(&unpacker, input_file_length);

  // We should have a single object, which is a map
  msgpack_unpacked result;
  msgpack_unpacked_init(&result);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);
  CU_ASSERT_EQUAL(result.data.type, MSGPACK_OBJECT_MAP);
  CU_ASSERT_EQUAL(result.data.via.map.size, 6);

  msgpack_object_kv *map_objects = result.data.via.map.ptr;

  // First object should be "x_size" => 42
  check_msgpack_key(&map_objects[0].key, "x_size");
  CU_ASSERT_EQUAL(map_objects[0].val.via.u64, 42);

  // Second object should be "y_size" => 23
  check_msgpack_key(&map_objects[1].key, "y_size");
  CU_ASSERT_EQUAL(map_objects[1].val.via.u64, 23);

  // Third object should be "max_entities" => 15
  check_msgpack_key(&map_objects[2].key, "max_entities");
  CU_ASSERT_EQUAL(map_objects[2].val.via.u64, 15);

  // Fourth object should be "last_index" => 2
  check_msgpack_key(&map_objects[3].key, "last_index");
  CU_ASSERT_EQUAL(map_objects[3].val.via.u64, 2);

  // Fifth index should be "entities", containing 2 entities
  // We're not checking the validity of the entities here, since that is already
  // tested elsewhere.
  check_msgpack_key(&map_objects[4].key, "entities");
  CU_ASSERT_EQUAL(map_objects[4].val.type, MSGPACK_OBJECT_ARRAY);
  CU_ASSERT_EQUAL(map_objects[4].val.via.array.size, 2);

  // Sixth index should be "items", containing 4 items
  // We're not checking the validity of the items here, since that is already
  // tested elsewhere.
  check_msgpack_key(&map_objects[5].key, "items");
  CU_ASSERT_EQUAL(map_objects[5].val.type, MSGPACK_OBJECT_ARRAY);
  CU_ASSERT_EQUAL(map_objects[5].val.via.array.size, 4);

  free(buffer);
  msgpack_sbuffer_destroy(&sbuffer);
  msgpack_unpacker_destroy(&unpacker);

  map_free(map);
}

void map_deserialize_test(void) {
  Map *map = create_serde_map();

  msgpack_sbuffer sbuffer;
  msgpack_sbuffer_init(&sbuffer);

  map_serialize(map, &sbuffer);
  CU_ASSERT_PTR_NOT_NULL(sbuffer.data);
  CU_ASSERT_FALSE(sbuffer.size == 0);

  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, 0);
  msgpack_unpacker_reserve_buffer(&unpacker, sbuffer.size);
  memcpy(msgpack_unpacker_buffer(&unpacker), sbuffer.data, sbuffer.size);
  msgpack_unpacker_buffer_consumed(&unpacker, sbuffer.size);

  msgpack_unpacked result;
  msgpack_unpacked_init(&result);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);

  Map          *deserialized = map_deserialize(&result.data.via.map);
  MapBoundaries original_boundaries = map_get_boundaries(map);
  MapBoundaries deser_boundaries = map_get_boundaries(deserialized);

  CU_ASSERT_EQUAL(original_boundaries.x, deser_boundaries.x);
  CU_ASSERT_EQUAL(original_boundaries.y, deser_boundaries.y);

  CU_ASSERT_EQUAL(map_count_items(map), map_count_items(deserialized));
  CU_ASSERT_EQUAL(map_count_entities(map), map_count_entities(deserialized));

  CU_ASSERT_PTR_NOT_NULL(map_get_item(deserialized, "A blue-jeans"));
  CU_ASSERT_PTR_NOT_NULL(map_get_entity(deserialized, "E2"));

  msgpack_unpacked_destroy(&result);
  msgpack_unpacker_destroy(&unpacker);
  map_free(deserialized);
  map_free(map);
}

void map_test_suite() {
  CU_pSuite suite = CU_add_suite("Map Tests", nullptr, nullptr);
  CU_add_test(suite, "Creation", &map_creation_test);
  CU_add_test(suite, "Handle Entities", &map_entities_test);
  CU_add_test(suite, "Handle Items", &map_items_test);
  CU_add_test(suite, "Msgpack Serialization", &map_serialization_test);
  CU_add_test(suite, "Msgpack Deserialization", &map_deserialize_test);
}

