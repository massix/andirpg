#include "item.h"
#include "point.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <asm-generic/fcntl.h>
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <msgpack/unpack.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void store_to_disk(const char *filename, msgpack_sbuffer *buffer) {
  int file_fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  write(file_fd, buffer->data, buffer->size);
  close(file_fd);
}

typedef struct ItemTypeTest {
  msgpack_sbuffer  *buffer;
  char             *filename;
  Item             *item;
  char             *loaded_buffer;
  size_t            loaded_buffer_size;
  bool              stored;
  msgpack_unpacked *unpacked;
} ItemTypeTest;

ItemTypeTest *item_type_test_new(const char *filename, Item *item) {
  ItemTypeTest *ret = calloc(1, sizeof(ItemTypeTest));
  ret->buffer = msgpack_sbuffer_new();
  ret->filename = strdup(filename);
  ret->item = item;
  ret->stored = false;
  ret->loaded_buffer = nullptr;
  ret->loaded_buffer_size = -1;
  ret->unpacked = nullptr;

  return ret;
}

void item_type_test_free(ItemTypeTest *itt) {
  free(itt->filename);
  msgpack_sbuffer_free(itt->buffer);
  item_free(itt->item);

  if (itt->stored) {
    unlink(itt->filename);
  }

  if (itt->loaded_buffer != nullptr) {
    free(itt->loaded_buffer);
  }

  if (itt->unpacked != nullptr) {
    msgpack_unpacked_destroy(itt->unpacked);
    free(itt->unpacked);
  }

  free(itt);
}

void item_type_test_serialize(ItemTypeTest *itt) {
  msgpack_sbuffer_init(itt->buffer);
  item_serialize(itt->item, itt->buffer);
}

void item_type_test_store(ItemTypeTest *itt) {
  store_to_disk(itt->filename, itt->buffer);
  itt->stored = true;
}

// Reload the serialization result from disk
void item_type_test_reload(ItemTypeTest *itt) {
  if (!itt->stored) {
    return;
  }

  FILE *input = fopen(itt->filename, "rb");
  itt->loaded_buffer_size = file_size(input);
  itt->loaded_buffer = (char *)malloc(itt->loaded_buffer_size);
  fread(itt->loaded_buffer, sizeof(char), itt->loaded_buffer_size, input);
  fclose(input);
}

void item_type_test_unpack(ItemTypeTest *self) {
  self->unpacked = calloc(1, sizeof(msgpack_unpacked));
  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, 0);
  msgpack_unpacker_reserve_buffer(&unpacker, self->loaded_buffer_size);
  memcpy(msgpack_unpacker_buffer(&unpacker), self->loaded_buffer, self->loaded_buffer_size);
  msgpack_unpacker_buffer_consumed(&unpacker, self->loaded_buffer_size);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, self->unpacked), MSGPACK_UNPACK_SUCCESS);

  msgpack_unpacker_destroy(&unpacker);
}

msgpack_object_kv *msgpack_map_get_key(msgpack_object_map *map, const char *key) {
  msgpack_object_kv *ret = nullptr;

  for (int i = 0; i < map->size; i++) {
    msgpack_object_str heading = map->ptr[i].key.via.str;
    if (strncmp(key, heading.ptr, heading.size) == 0) {
      ret = &map->ptr[i];
    }
  }

  return ret;
}

void item_type_test_complete(ItemTypeTest *self) {
  item_type_test_serialize(self);
  CU_ASSERT_PTR_NOT_NULL(self->buffer->data);

  item_type_test_store(self);
  CU_ASSERT_TRUE(self->stored);

  item_type_test_reload(self);
  CU_ASSERT_PTR_NOT_NULL(self->loaded_buffer);
  CU_ASSERT_TRUE(self->loaded_buffer_size > 0);

  item_type_test_unpack(self);
  CU_ASSERT_PTR_NOT_NULL(self->unpacked);
  CU_ASSERT_EQUAL(self->unpacked->data.type, MSGPACK_OBJECT_MAP);
  CU_ASSERT_EQUAL(self->unpacked->data.via.map.size, 6);

  // Check common fields
  msgpack_object_kv *kv_type = msgpack_map_get_key(&self->unpacked->data.via.map, "type");
  CU_ASSERT_PTR_NOT_NULL(kv_type);
  CU_ASSERT_EQUAL(kv_type->val.via.u64, item_get_type(self->item));

  msgpack_object_kv *kv_name = msgpack_map_get_key(&self->unpacked->data.via.map, "name");
  CU_ASSERT_PTR_NOT_NULL(kv_name);
  CU_ASSERT_EQUAL(strncmp(kv_name->val.via.str.ptr, item_get_name(self->item), kv_name->val.via.str.size), 0);

  msgpack_object_kv *kv_weight = msgpack_map_get_key(&self->unpacked->data.via.map, "weight");
  CU_ASSERT_PTR_NOT_NULL(kv_weight);
  CU_ASSERT_EQUAL(kv_weight->val.via.u64, item_get_weight(self->item));

  msgpack_object_kv *kv_value = msgpack_map_get_key(&self->unpacked->data.via.map, "value");
  CU_ASSERT_PTR_NOT_NULL(kv_value);
  CU_ASSERT_EQUAL(kv_value->val.via.u64, item_get_value(self->item));

  // This will be checked by the caller
  CU_ASSERT_PTR_NOT_NULL(msgpack_map_get_key(&self->unpacked->data.via.map, "properties"));

  msgpack_object_kv *kv_coords = msgpack_map_get_key(&self->unpacked->data.via.map, "coords");
  CU_ASSERT_PTR_NOT_NULL(kv_coords);
  if (item_has_coords(self->item)) {
    CU_ASSERT_EQUAL(kv_coords->val.via.array.size, 2);
    CU_ASSERT_EQUAL(kv_coords->val.via.array.ptr[0].via.u64, point_get_x(item_get_coords(self->item)));
    CU_ASSERT_EQUAL(kv_coords->val.via.array.ptr[1].via.u64, point_get_y(item_get_coords(self->item)));
  } else {
    CU_ASSERT_EQUAL(kv_coords->val.via.array.size, 0);
  }
}

msgpack_object_kv *item_type_test_get_properties(ItemTypeTest *self) {
  if (self->unpacked != nullptr) {
    return msgpack_map_get_key(&self->unpacked->data.via.map, "properties");
  }

  return nullptr;
}

void item_serialize_test(void) {
  Item *armor = armor_new("Armor 1", 20, 512, 20, 5, 3);
  Item *pickaxe = tool_new("Pickaxe", 30, 10, 2, 40);
  Item *weapon = weapon_new("Excalibur", 20, 50, 2, 30, 10);
  Item *generic_item = item_new(FORAGE, "A pear", 10, 3);
  item_set_coords(weapon, 10, 30);

  ItemTypeTest *armor_test = item_type_test_new("./armor.bin", armor);
  ItemTypeTest *pickaxe_test = item_type_test_new("./pickaxe.bin", pickaxe);
  ItemTypeTest *weapon_test = item_type_test_new("./weapon.bin", weapon);
  ItemTypeTest *generic_test = item_type_test_new("./generic.bin", generic_item);

  item_type_test_complete(armor_test);
  item_type_test_complete(pickaxe_test);
  item_type_test_complete(weapon_test);
  item_type_test_complete(generic_test);

  msgpack_object_kv *armor_properties = item_type_test_get_properties(armor_test);
  CU_ASSERT_PTR_NOT_NULL(armor_properties);
  CU_ASSERT_EQUAL(armor_properties->val.type, MSGPACK_OBJECT_MAP)
  CU_ASSERT_EQUAL(msgpack_map_get_key(&armor_properties->val.via.map, "defense_value")->val.via.u64,
                  armor_get_defense_value(item_get_properties(armor)));
  CU_ASSERT_EQUAL(msgpack_map_get_key(&armor_properties->val.via.map, "life_points")->val.via.u64,
                  armor_get_life_points(item_get_properties(armor)));
  CU_ASSERT_EQUAL(msgpack_map_get_key(&armor_properties->val.via.map, "armor_class")->val.via.u64,
                  armor_get_armor_class(item_get_properties(armor)));

  msgpack_object_kv *pickaxe_properties = item_type_test_get_properties(pickaxe_test);
  CU_ASSERT_PTR_NOT_NULL(pickaxe_properties);
  CU_ASSERT_EQUAL(pickaxe_properties->val.type, MSGPACK_OBJECT_MAP);
  CU_ASSERT_EQUAL(msgpack_map_get_key(&pickaxe_properties->val.via.map, "hands")->val.via.u64,
                  tool_get_hands(item_get_properties(pickaxe)));
  CU_ASSERT_EQUAL(msgpack_map_get_key(&pickaxe_properties->val.via.map, "life_points")->val.via.u64,
                  tool_get_life_points(item_get_properties(pickaxe)));

  msgpack_object_kv *weapon_properties = item_type_test_get_properties(weapon_test);
  CU_ASSERT_PTR_NOT_NULL(weapon_properties);
  CU_ASSERT_EQUAL(weapon_properties->val.type, MSGPACK_OBJECT_MAP);
  CU_ASSERT_EQUAL(msgpack_map_get_key(&weapon_properties->val.via.map, "hands")->val.via.u64,
                  weapon_get_hands(item_get_properties(weapon)));
  CU_ASSERT_EQUAL(msgpack_map_get_key(&weapon_properties->val.via.map, "life_points")->val.via.u64,
                  weapon_get_life_points(item_get_properties(weapon)));
  CU_ASSERT_EQUAL(msgpack_map_get_key(&weapon_properties->val.via.map, "attack_power")->val.via.u64,
                  weapon_get_attack_power(item_get_properties(weapon)));

  // Generic items should not have properties (yet)
  msgpack_object_kv *generic_properties = item_type_test_get_properties(generic_test);
  CU_ASSERT_PTR_NOT_NULL(generic_properties);
  CU_ASSERT_EQUAL(generic_properties->val.type, MSGPACK_OBJECT_NIL);

  item_type_test_free(generic_test);
  item_type_test_free(weapon_test);
  item_type_test_free(pickaxe_test);
  item_type_test_free(armor_test);
}

#define ASSERT_ITEM_COMMON(prop, lhs, rhs) \
  { CU_ASSERT_EQUAL(item_get_##prop(lhs), item_get_##prop(rhs)); }

#define ASSERT_ITEM_PROP(fn, lhs, rhs) \
  { CU_ASSERT_EQUAL(fn(item_get_properties(lhs)), fn(item_get_properties(rhs))); }

void item_deserialize_test(void) {
  Item *armor = armor_new("Armor 1", 20, 512, 20, 5, 3);
  Item *pickaxe = tool_new("Pickaxe", 30, 10, 2, 40);
  Item *weapon = weapon_new("Excalibur", 20, 50, 2, 30, 10);
  item_set_coords(weapon, 10, 30);

  Item **all_items = calloc(3, sizeof(Item *));
  all_items[0] = armor;
  all_items[1] = pickaxe;
  all_items[2] = weapon;

  Item **all_deserialized = calloc(3, sizeof(Item *));

  for (uint8_t i = 0; i < 3; i++) {
    Item            *current_item = all_items[i];
    msgpack_sbuffer *sbuffer = msgpack_sbuffer_new();
    item_serialize(current_item, sbuffer);

    msgpack_unpacker unpacker;
    msgpack_unpacker_init(&unpacker, 0);
    msgpack_unpacker_reserve_buffer(&unpacker, sbuffer->size);
    memcpy(msgpack_unpacker_buffer(&unpacker), sbuffer->data, sbuffer->size);
    msgpack_unpacker_buffer_consumed(&unpacker, sbuffer->size);

    // Unpack the object now
    msgpack_unpacked result;
    msgpack_unpacked_init(&result);

    CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);

    Item *deserialized = item_deserialize(&result.data.via.map);
    all_deserialized[i] = deserialized;

    ASSERT_ITEM_COMMON(value, deserialized, current_item);
    ASSERT_ITEM_COMMON(type, deserialized, current_item);
    ASSERT_ITEM_COMMON(weight, deserialized, current_item);
    CU_ASSERT_TRUE(strings_equal(item_get_name(deserialized), item_get_name(current_item)));

    msgpack_sbuffer_free(sbuffer);
    msgpack_unpacker_destroy(&unpacker);
  }

  // Armor tests
  ASSERT_ITEM_PROP(armor_get_armor_class, all_deserialized[0], all_items[0]);
  ASSERT_ITEM_PROP(armor_get_defense_value, all_deserialized[0], all_items[0]);
  ASSERT_ITEM_PROP(armor_get_life_points, all_deserialized[0], all_items[0]);

  // Tool tests
  ASSERT_ITEM_PROP(tool_get_hands, all_deserialized[1], all_items[1]);
  ASSERT_ITEM_PROP(tool_get_life_points, all_deserialized[1], all_items[1]);

  // Weapon tests
  ASSERT_ITEM_PROP(weapon_get_hands, all_deserialized[2], all_items[2]);
  ASSERT_ITEM_PROP(weapon_get_life_points, all_deserialized[2], all_items[2]);
  ASSERT_ITEM_PROP(weapon_get_attack_power, all_deserialized[2], all_items[2]);

  for (uint8_t i = 0; i < 3; i++) {
    item_free(all_deserialized[i]);
    item_free(all_items[i]);
  }

  free(all_items);
  free(all_deserialized);
}

void item_test_suite() {
  CU_pSuite suite = CU_add_suite("Items Tests", nullptr, nullptr);
  CU_add_test(suite, "Item creation", &item_new_test);
  CU_add_test(suite, "Item properties", &item_properties_test);
  CU_add_test(suite, "Item coordinates", &item_coordinates_test);
  CU_add_test(suite, "Item clonation", &item_clone_test);
  CU_add_test(suite, "Item serialization", &item_serialize_test);
  CU_add_test(suite, "Item deserialization", &item_deserialize_test);
}

