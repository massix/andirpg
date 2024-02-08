#include "engine.h"
#include "entity.h"
#include "map.h"
#include "point.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void engine_creation_test(void) {
  Engine *engine = engine_new(map_new(20, 20, 10));
  CU_ASSERT_PTR_NOT_NULL(engine);

  engine_free(engine);
}

void engine_entities_test(void) {
  Map    *map = map_new(20, 20, 10);
  Engine *engine = engine_new(map);
  engine_add_entity(engine, entity_new(30, HUMAN, "e1", 0, 0));

  CU_ASSERT_EQUAL(map_count_entities(map), 1);
  CU_ASSERT_TRUE(map_contains_entity(map, "e1"));

  CU_ASSERT_FALSE(engine_has_active_entity(engine));
  engine_set_active_entity(engine, "e1");
  CU_ASSERT_TRUE(engine_has_active_entity(engine));

  engine_set_active_entity(engine, "notexisting");
  CU_ASSERT_FALSE(engine_has_active_entity(engine));

  engine_set_active_entity(engine, "e1");
  engine_clear_active_entity(engine);
  CU_ASSERT_FALSE(engine_has_active_entity(engine));

  engine_free(engine);
}

void engine_close_entities_test(void) {
  Engine *engine = engine_new(map_new(20, 20, 8));
  engine_add_entity(engine, entity_new(10, INHUMAN, "z1", 0, 1));
  engine_add_entity(engine, entity_new(10, HUMAN, "h1", 1, 1));
  engine_add_entity(engine, entity_new(10, INHUMAN, "z2", 2, 1));
  engine_add_entity(engine, entity_new(10, INHUMAN, "z3", 1, 0));
  engine_set_active_entity(engine, "h1");

  // This entity is far from the active one
  engine_add_entity(engine, entity_new(10, HUMAN, "far away", 4, 0));

  ssize_t  size;
  Entity **close_entities = engine_get_close_entities(engine, &size);
  CU_ASSERT_PTR_NOT_NULL(close_entities);

  CU_ASSERT_EQUAL(size, 3);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(close_entities[0]), "z1"));
  CU_ASSERT_TRUE(strings_equal(entity_get_name(close_entities[1]), "z2"));
  CU_ASSERT_TRUE(strings_equal(entity_get_name(close_entities[2]), "z3"));

  free(close_entities);
  engine_free(engine);
}

void engine_keypress_test(void) {
  Engine *engine = engine_new(map_new(5, 5, 10));
  Entity *active = entity_new(30, ANIMAL, "e4", 3, 0);
  engine_add_entity(engine, entity_new(30, HUMAN, "e1", 0, 0));
  engine_add_entity(engine, entity_new(30, ANIMAL, "e2", 1, 0));
  engine_add_entity(engine, entity_new(30, INHUMAN, "e3", 2, 0));
  engine_add_entity(engine, active);
  engine_add_entity(engine, entity_new(30, TREE, "e5", 0, 1));
  engine_add_entity(engine, entity_new(30, MOUNTAIN, "e6", 1, 1));
  engine_add_entity(engine, entity_new(30, HUMAN, "e7", 2, 1));

  /*
    @#&#.
    %^@..
    .....
    .....
    .....
  */

  engine_set_active_entity(engine, "e4");

  engine_handle_keypress(engine, 'l');
  /*
    @#&.#
    %^@..
    .....
    .....
    .....
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 4);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 0);

  engine_handle_keypress(engine, 'j');
  /*
    @#&..
    %^@.#
    .....
    .....
    .....
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 4);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 1);

  engine_handle_keypress(engine, 'h');
  /*
    @#&..
    %^@#.
    .....
    .....
    .....
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 3);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 1);

  // Impossible movement, tile is not free!
  engine_handle_keypress(engine, 'h');
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 3);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 1);

  engine_handle_keypress(engine, 'j');
  engine_handle_keypress(engine, 'j');
  engine_handle_keypress(engine, 'j');
  /*
    @#&..
    %^@..
    .....
    .....
    ...#.
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 3);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 4);

  // Impossible movement, out of bounds!
  engine_handle_keypress(engine, 'j');
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 3);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 4);

  engine_handle_keypress(engine, 'h');
  engine_handle_keypress(engine, 'h');
  engine_handle_keypress(engine, 'h');
  /*
    @#&..
    %^@..
    .....
    .....
    #....
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 0);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 4);

  // Impossible movement, out of bounds!
  engine_handle_keypress(engine, 'h');
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 0);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 4);

  engine_handle_keypress(engine, 'l');
  engine_handle_keypress(engine, 'l');
  engine_handle_keypress(engine, 'l');
  engine_handle_keypress(engine, 'l');
  /*
    @#&..
    %^@..
    .....
    .....
    ....#
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 4);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 4);

  // Impossible movement, out of bounds!
  engine_handle_keypress(engine, 'l');
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 4);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 4);

  engine_handle_keypress(engine, 'k');
  engine_handle_keypress(engine, 'k');
  engine_handle_keypress(engine, 'k');
  engine_handle_keypress(engine, 'k');
  /*
    @#&.#
    %^@..
    .....
    .....
    .....
  */
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 4);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 0);

  // Impossible movement, out of bounds!
  engine_handle_keypress(engine, 'k');
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(active)), 4);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(active)), 0);
  engine_free(engine);
}

void engine_attack_test(void) {
  Engine *engine = engine_new(map_new(20, 20, 10));
  // These two entities are close, they can attack each other
  Entity *human1 = entity_new(10, HUMAN, "h1", 10, 10);
  Entity *zombie = entity_new(8, INHUMAN, "z1", 9, 11);
  engine_add_entity(engine, human1);
  engine_add_entity(engine, zombie);

  // This entity is close to "z1" but not to "h1"
  Entity *human2 = entity_new(6, HUMAN, "h2", 8, 11);
  engine_add_entity(engine, human2);

  // This entity is far from everything
  Entity *human3 = entity_new(15, HUMAN, "h3", 12, 11);
  engine_add_entity(engine, human3);

  // Successful attack
  engine_entity_attack(engine, human1, zombie);
  CU_ASSERT_EQUAL(entity_get_life_points(zombie), 7);

  // Same
  engine_entity_attack(engine, zombie, human1);
  CU_ASSERT_EQUAL(entity_get_life_points(human1), 9);

  // Unsuccessful attack (entities are not close to each other)
  engine_entity_attack(engine, human1, human2);
  CU_ASSERT_EQUAL(entity_get_life_points(human2), 6);

  // Successful attack
  engine_entity_attack(engine, zombie, human2);
  CU_ASSERT_EQUAL(entity_get_life_points(human2), 5);

  // No one can attack human3 and they can't attack anyone
  engine_entity_attack(engine, zombie, human3);
  engine_entity_attack(engine, human1, human3);
  engine_entity_attack(engine, human2, human3);
  CU_ASSERT_EQUAL(entity_get_life_points(human3), 15);

  // But human can get close and personal with the zombie!
  entity_move(human3, -3, 1); // should arrive at 9,12
  engine_entity_attack(engine, human3, zombie);
  CU_ASSERT_EQUAL(entity_get_life_points(zombie), 6);

  engine_free(engine);
}

void engine_test_suite() {
  CU_pSuite suite = CU_add_suite("Engine Tests", nullptr, nullptr);
  CU_add_test(suite, "Engine creation", &engine_creation_test);
  CU_add_test(suite, "Engine close entities", &engine_close_entities_test);
  CU_add_test(suite, "Engine entities", &engine_entities_test);
  CU_add_test(suite, "Engine keypress", &engine_keypress_test);
  CU_add_test(suite, "Engine attacks", &engine_attack_test);
}

