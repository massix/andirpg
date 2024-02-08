#include "item.h"
#include "point.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

void item_new_test(void) {
  const char *item_name = "A broadsword";
  Item       *weapon = item_new(WEAPON, item_name, 10, 512);
  CU_ASSERT_PTR_NOT_NULL(weapon);
  CU_ASSERT_FALSE(item_has_properties(weapon));
  CU_ASSERT_TRUE(strings_equal(item_get_name(weapon), item_name));
  CU_ASSERT_EQUAL(item_get_type(weapon), WEAPON);

  // Also verify that the pointer has been copied over
  CU_ASSERT_TRUE(item_name != item_get_name(weapon));

  CU_ASSERT_EQUAL(item_get_weight(weapon), 10);
  CU_ASSERT_EQUAL(item_get_value(weapon), 512);

  Item *cloned = item_clone(weapon);
  CU_ASSERT_PTR_NOT_NULL(cloned);
  CU_ASSERT_FALSE(item_has_properties(cloned));
  CU_ASSERT_FALSE(item_has_coords(cloned));

  item_free(cloned);
  item_free(weapon);
}

void item_clone_test(void) {
  Item *weapon = weapon_new("Some weapon", 10, 30, 1, 50, 10);
  Item *cloned_weapon = item_clone(weapon);

  CU_ASSERT_TRUE(item_has_properties(cloned_weapon));
  CU_ASSERT_FALSE(item_has_coords(cloned_weapon));

  WeaponProperties *properties = item_get_properties(cloned_weapon);
  CU_ASSERT_EQUAL(weapon_get_hands(properties), 1);
  CU_ASSERT_EQUAL(weapon_get_life_points(properties), 10);
  CU_ASSERT_EQUAL(weapon_get_attack_power(properties), 50);

  item_set_coords(cloned_weapon, 10, 12);
  CU_ASSERT_TRUE(item_has_coords(cloned_weapon));
  CU_ASSERT_FALSE(item_has_coords(weapon));

  Item *second_clone = item_clone(cloned_weapon);
  CU_ASSERT_TRUE(item_has_coords(second_clone));
  CU_ASSERT_TRUE(item_has_properties(second_clone));

  item_free(second_clone);
  item_free(weapon);
  item_free(cloned_weapon);
}

void item_properties_test(void) {
  Item *item1 = weapon_new("Some weapon", 10, 30, 2, 10, 30);
  CU_ASSERT_TRUE(item_has_properties(item1));

  WeaponProperties *properties = item_get_properties(item1);
  CU_ASSERT_EQUAL(weapon_get_hands(properties), 2);
  CU_ASSERT_EQUAL(weapon_get_life_points(item_get_properties(item1)), 30);
  CU_ASSERT_EQUAL(weapon_get_attack_power(item_get_properties(item1)), 10);

  Item *item2 = tool_new("Some tool", 15, 500, 1, 30);
  CU_ASSERT_EQUAL(tool_get_hands(item_get_properties(item2)), 1);
  CU_ASSERT_EQUAL(tool_get_life_points(item_get_properties(item2)), 30);

  Item *item3 = armor_new("Some armor", 30, 300, 15, 4, 3);
  CU_ASSERT_EQUAL(armor_get_armor_class(item_get_properties(item3)), 3);
  CU_ASSERT_EQUAL(armor_get_life_points(item_get_properties(item3)), 4);
  CU_ASSERT_EQUAL(armor_get_defense_value(item_get_properties(item3)), 15);

  item_free(item3);
  item_free(item2);
  item_free(item1);
}

void item_coordinates_test(void) {
  Item *without_coords = item_new(WEAPON, "a weapon", 10, 30);
  CU_ASSERT_FALSE(item_has_coords(without_coords));

  item_set_coords(without_coords, 10, 12);
  CU_ASSERT_TRUE(item_has_coords(without_coords));

  Point *coords = item_get_coords(without_coords);
  CU_ASSERT_EQUAL(point_get_x(coords), 10);
  CU_ASSERT_EQUAL(point_get_y(coords), 12);

  item_clear_coords(without_coords);
  CU_ASSERT_FALSE(item_has_coords(without_coords));

  item_free(without_coords);
}

void item_test_suite() {
  CU_pSuite suite = CU_add_suite("Items Tests", nullptr, nullptr);
  CU_add_test(suite, "Item creation", &item_new_test);
  CU_add_test(suite, "Item properties", &item_properties_test);
  CU_add_test(suite, "Item coordinates", &item_coordinates_test);
  CU_add_test(suite, "Item clonation", &item_clone_test);
}

