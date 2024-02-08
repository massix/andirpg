#include "item.h"
#include "point.h"
#include "utils.h"
#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <entity.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void entity_creation_test(void) {
  Entity *human = entity_new(30, HUMAN, "Avatar", 20, 30);

  CU_ASSERT_TRUE(strings_equal(entity_get_name(human), "Avatar"));
  CU_ASSERT_EQUAL(*entity_get_entity_type(human), HUMAN);
  CU_ASSERT_TRUE(strings_equal(entity_type_to_string(*entity_get_entity_type(human)), "Human"));
  CU_ASSERT_EQUAL(entity_get_life_points(human), 30);
  CU_ASSERT_EQUAL(*entity_get_entity_type(human), HUMAN);

  Point *entity_coords = entity_get_coords(human);
  CU_ASSERT_EQUAL(point_get_x(entity_coords), 20);
  CU_ASSERT_EQUAL(point_get_y(entity_coords), 30);

  entity_free(human);
}

void entity_lifepoints_test(void) {
  Entity *monster = entity_new(15, INHUMAN, "Orc", 20, 20);
  CU_ASSERT_EQUAL(entity_get_life_points(monster), 15);

  // Can hurt a creature
  entity_hurt(monster, 12);
  CU_ASSERT_TRUE(entity_is_alive(monster));
  CU_ASSERT_EQUAL(entity_get_life_points(monster), 3);

  // Can heal a creature
  entity_heal(monster, 10);
  CU_ASSERT_TRUE(entity_is_alive(monster));
  CU_ASSERT_EQUAL(entity_get_life_points(monster), 13);

  // Life can never go past the starting value
  entity_heal(monster, 4);
  CU_ASSERT_TRUE(entity_is_alive(monster));
  CU_ASSERT_EQUAL(entity_get_life_points(monster), 15);

  // Lifepoints should never go under 0
  entity_hurt(monster, 20);
  CU_ASSERT_FALSE(entity_is_alive(monster));
  CU_ASSERT_TRUE(entity_is_dead(monster));
  CU_ASSERT_EQUAL(entity_get_life_points(monster), 0);

  // Cannot heal a dead entity
  entity_heal(monster, 10);
  CU_ASSERT_FALSE(entity_is_alive(monster));
  CU_ASSERT_EQUAL(entity_get_life_points(monster), 0);

  CU_ASSERT_EQUAL(entity_get_starting_life_points(monster), 15);

  entity_free(monster);
}

void entity_resurrect_test(void) {
  Entity *human = entity_new(15, HUMAN, "human", 0, 0);
  Entity *inhuman = entity_new(15, INHUMAN, "inhuman", 0, 0);
  entity_hurt(human, 15);
  entity_hurt(inhuman, 15);

  CU_ASSERT_TRUE(entity_is_dead(human));
  CU_ASSERT_TRUE(entity_is_dead(inhuman));

  // Only inhumans can resurrect
  entity_resurrect(human);
  CU_ASSERT_TRUE(entity_is_dead(human));

  entity_resurrect(inhuman);
  CU_ASSERT_FALSE(entity_is_dead(inhuman));
  CU_ASSERT_EQUAL(entity_get_life_points(inhuman), 15);

  entity_free(human);
  entity_free(inhuman);
}

void entity_move_test(void) {
  Entity *dog = entity_new(30, ANIMAL, "Bounty", 46, -8);
  CU_ASSERT_TRUE(entity_can_move(dog));

  entity_move(dog, -4, 20);

  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(dog)), 42);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(dog)), 12);

  Entity *tree = entity_new(1, TREE, "Ent", 20, 1);
  CU_ASSERT_FALSE(entity_can_move(tree));
  entity_move(tree, 10, 14);
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(tree)), 20);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(tree)), 1);

  Entity *mountain = entity_new(1, MOUNTAIN, "Fuji", 10, 14);
  CU_ASSERT_FALSE(entity_can_move(mountain));

  // Dead entities cannot move
  entity_hurt(dog, 30);
  entity_move(dog, 1, 1);
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(dog)), 42);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(dog)), 12);

  entity_free(dog);
  entity_free(tree);
  entity_free(mountain);
}

void entity_to_string_test(void) {
  Entity *entity = entity_new(512, INHUMAN, "A vampire", 21, 42);
  entity_move(entity, 21, -2);
  char *str = entity_to_string(entity);

  CU_ASSERT_TRUE(strings_equal(str, "512:&:42:40:A vampire"));

  Entity *rebuilt = entity_from_string(str);
  CU_ASSERT_EQUAL(entity_get_life_points(rebuilt), 512);
  CU_ASSERT_EQUAL(*entity_get_entity_type(rebuilt), INHUMAN);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(rebuilt), "A vampire"));
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(rebuilt)), 42);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(rebuilt)), 40);

  free(str);
}

bool filter_weapons(Item *item) {
  return strncmp(item_get_name(item), "Weapon", 6) == 0;
}

bool filter_tools(Item *item) {
  return item_get_type(item) == TOOL;
}

bool filter_none(Item *) {
  return false;
}

void entity_inventory_test(void) {
  Entity *entity = entity_new(20, HUMAN, "An human", 0, 0);
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 0);

  entity_inventory_add_item(entity, weapon_new("Weapon number one", 30, 20, 1, 10, 10));
  entity_inventory_add_item(entity, tool_new("Key", 2, 1, 1, 30));
  entity_inventory_add_item(entity, weapon_new("Weapon two", 30, 20, 1, 10, 10));
  entity_inventory_add_item(entity, tool_new("Pickaxe", 50, 1, 1, 30));
  entity_inventory_add_item(entity, weapon_new("Weapon 3", 30, 20, 1, 10, 10));
  entity_inventory_add_item(entity, tool_new("Heal Potion", 2, 100, 1, 30));
  entity_inventory_add_item(entity, weapon_new("Weapon: a broadsword", 30, 20, 1, 10, 10));
  entity_inventory_add_item(entity, armor_new("An armor I guess", 50, 500, 20, 30, 4));
  entity_inventory_add_item(entity, weapon_new("Weapn with a typo (should not be filtered)", 30, 20, 1, 10, 10));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 9);

  ssize_t retrieved_weapons = 0;
  Item  **weapons = entity_inventory_filter(entity, &filter_weapons, &retrieved_weapons);
  CU_ASSERT_EQUAL(retrieved_weapons, 4);

  ssize_t retrieved_tools = 0;
  Item  **tools = entity_inventory_filter(entity, &filter_tools, &retrieved_tools);
  CU_ASSERT_EQUAL(retrieved_tools, 3);

  ssize_t retrieved_none = -1;
  Item  **nothing = entity_inventory_filter(entity, &filter_none, &retrieved_none);
  CU_ASSERT_EQUAL(retrieved_none, 0);

  Item **all_inventory = entity_inventory_get(entity);

  // Remove item in the middle
  entity_inventory_remove_item(entity, "Heal Potion");
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 8);
  CU_ASSERT_TRUE(strings_equal(item_get_name(all_inventory[5]), "Weapn with a typo (should not be filtered)"));

  // Remove first item
  entity_inventory_remove_item(entity, "Weapon number one");
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 7);
  CU_ASSERT_TRUE(strings_equal(item_get_name(all_inventory[0]), "An armor I guess"));

  // Remove last item
  entity_inventory_remove_item(entity, "Weapn with a typo (should not be filtered)");
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 6);
  CU_ASSERT_TRUE(strings_equal(item_get_name(all_inventory[5]), "Weapon: a broadsword"));

  entity_inventory_clear(entity);
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 0);

  Entity *empty_inventory = entity_new(10, TREE, "A tree", 0, 0);
  entity_inventory_remove_item(empty_inventory, "Not existing");
  CU_ASSERT_EQUAL(entity_inventory_count(empty_inventory), 0);

  free(nothing);
  free(tools);
  free(weapons);
  entity_free(empty_inventory);
  entity_free(entity);
}

void entity_test_suite() {
  CU_pSuite suite = CU_add_suite("Entity Tests", nullptr, nullptr);
  CU_add_test(suite, "Create a basic entity", &entity_creation_test);
  CU_add_test(suite, "Manipulate life points", &entity_lifepoints_test);
  CU_add_test(suite, "Move entities", &entity_move_test);
  CU_add_test(suite, "Resurrect entities", &entity_resurrect_test);
  CU_add_test(suite, "Serialization", &entity_to_string_test);
  CU_add_test(suite, "Inventory manipulation", &entity_inventory_test);
}

