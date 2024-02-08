#include "item.h"
#include "map.h"
#include "point.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <entity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void map_dump_test(void) {
  Map *map = map_new(20, 50, 16);
  map_add_entity(map, entity_new(20, TREE, "Just a tree", 15, 21));
  map_add_entity(map, entity_new(15, HUMAN, "Just a human", 14, 21));
  map_add_entity(map, entity_new(47, ANIMAL, "Just an animal", 14, 22));
  map_add_entity(map, entity_new(22, MOUNTAIN, "Just a mountain", 13, 21));
  map_add_entity(map, entity_new(22, MOUNTAIN, "Delete me", 10, 21));
  map_add_entity(map, entity_new(21, TREE, "Another tree", 11, 21));
  map_add_entity(map, entity_new(17, ANIMAL, "Another animal", 20, 24));

  map_remove_entity(map, "Delete me");
  entity_move(map_get_entity(map, "Another animal"), 3, -4);

  const char *dump_file = "./test-dump-map";

  map_dump_to_file(map, dump_file);

  FILE *result = fopen(dump_file, "r");

  // First line contains the raw values
  size_t read_bytes;
  char  *read_line = nullptr;
  getline(&read_line, &read_bytes, result);
  read_line[strcspn(read_line, "\n")] = '\0';
  CU_ASSERT(read_bytes > 0);
  CU_ASSERT_PTR_NOT_NULL(read_line);
  CU_ASSERT_TRUE(strings_equal(read_line, "20:50:6:16"));

  Map          *rebuilt = map_from_string(read_line);
  MapBoundaries rebuilt_boundaries = map_get_boundaries(rebuilt);
  CU_ASSERT_EQUAL(rebuilt_boundaries.x, 20);
  CU_ASSERT_EQUAL(rebuilt_boundaries.y, 50);
  free(read_line);

  // Then, we have all the entities
  char **all_entities = calloc(50, sizeof(char *));
  for (size_t i = 0; i < 50; i++) {
    all_entities[i] = calloc(1024, sizeof(char));
  }

  read_line = nullptr;
  size_t entity_index = 0;
  while (getline(&read_line, &read_bytes, result) != -1) {
    char *target = all_entities[entity_index];
    strncpy(target, read_line, read_bytes);
    target[strcspn(target, "\n")] = '\0';

    free(read_line);
    read_line = nullptr;
    entity_index++;
  }

  // Here I can rebuild the whole map!
  for (size_t i = 0; i < 50; i++) {
    if (strlen(all_entities[i])) {
      Entity *ent = entity_from_string(all_entities[i]);
      CU_ASSERT_PTR_NOT_NULL(ent);
      map_add_entity(rebuilt, ent);
    }

    free(all_entities[i]);
  }

  free(all_entities);

  // Time to check!
  CU_ASSERT_EQUAL(map_count_entities(rebuilt), 6);
  CU_ASSERT_TRUE(map_contains_entity(rebuilt, "Just a tree"));
  CU_ASSERT_TRUE(map_contains_entity(rebuilt, "Just a human"));
  CU_ASSERT_TRUE(map_contains_entity(rebuilt, "Just an animal"));
  CU_ASSERT_TRUE(map_contains_entity(rebuilt, "Just a mountain"));
  CU_ASSERT_TRUE(map_contains_entity(rebuilt, "Another tree"));
  CU_ASSERT_TRUE(map_contains_entity(rebuilt, "Another animal"));
  CU_ASSERT_FALSE(map_contains_entity(rebuilt, "Delete mw"));

  Point *coords = entity_get_coords(map_get_entity(rebuilt, "Another animal"));
  CU_ASSERT_EQUAL(point_get_x(coords), 23);
  CU_ASSERT_EQUAL(point_get_y(coords), 20);

  fclose(result);
  unlink(dump_file);
  map_free(map);
  map_free(rebuilt);
}

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

void map_draw_test(void) {
  const char *test_file = "test.txt";
  Map        *map = map_new(5, 5, 10);

  // Populate the map
  map_add_entity(map, entity_new(30, HUMAN, "h1", 2, 0));
  map_add_entity(map, entity_new(30, INHUMAN, "v1", 1, 1));
  map_add_entity(map, entity_new(30, TREE, "t1", 0, 0));
  map_add_entity(map, entity_new(30, ANIMAL, "a1", 3, 4));
  map_add_entity(map, entity_new(10, INHUMAN, "v2", 0, 2));
  map_add_entity(map, entity_new(1, MOUNTAIN, "m1", 2, 2));
  map_add_entity(map, entity_new(1, MOUNTAIN, "m2", 3, 2));
  map_add_entity(map, entity_new(1, MOUNTAIN, "m2", 4, 2));

  FILE *input_file = fopen(test_file, "w");
  char *content = calloc(1024, sizeof(char));
  map_fdraw(map, input_file);
  fflush(input_file);
  fclose(input_file);

  input_file = fopen(test_file, "r");
  fread(content, sizeof(char), 1024, input_file);
  CU_ASSERT_TRUE(strings_equal(content, "%.@..\n"
                                        ".&...\n"
                                        "&.^^^\n"
                                        ".....\n"
                                        "...#.\n"));
  fclose(input_file);

  unlink(test_file);
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

void map_test_suite() {
  CU_pSuite suite = CU_add_suite("Map Tests", nullptr, nullptr);
  CU_add_test(suite, "Creation", &map_creation_test);
  CU_add_test(suite, "Handle Entities", &map_entities_test);
  CU_add_test(suite, "Handle Items", &map_items_test);
  CU_add_test(suite, "Draw on screen", &map_draw_test);
  CU_add_test(suite, "Serialization", &map_dump_test);
}

