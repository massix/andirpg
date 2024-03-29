#include "item.h"
#include "perk.h"
#include "point.h"
#include "serde.h"
#include "utils.h"
#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
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

#define CU_ASSERT_MAP_KEY(k, header)                                               \
  {                                                                                \
    CU_ASSERT_EQUAL((k).key.type, MSGPACK_OBJECT_STR)                              \
    CU_ASSERT_EQUAL(strncmp((k).key.via.str.ptr, header, (k).key.via.str.size), 0) \
  }

void entity_creation_test(void) {
  Entity *human = entity_build(30, HUMAN, "Avatar", 20, 30);

  CU_ASSERT_TRUE(strings_equal(entity_get_name(human), "Avatar"));
  CU_ASSERT_EQUAL(entity_get_entity_type(human), HUMAN);
  CU_ASSERT_EQUAL(entity_get_life_points(human), 30);
  CU_ASSERT_EQUAL(entity_get_entity_type(human), HUMAN);

  Point const *entity_coords = entity_get_coords(human);
  CU_ASSERT_EQUAL(point_get_x(entity_coords), 20);
  CU_ASSERT_EQUAL(point_get_y(entity_coords), 30);

  CU_ASSERT_EQUAL(entity_get_seeing_distance(human), 10);
  CU_ASSERT_EQUAL(entity_get_hearing_distance(human), 10);

#define test_get_set(prop, val)  \
  entity_set_##prop(human, val); \
  CU_ASSERT_EQUAL(entity_get_##prop(human), val);

  test_get_set(xp, 42);
  test_get_set(current_level, 10);
  test_get_set(thirst, 5);
  test_get_set(hunger, 42);
  test_get_set(tiredness, 98);

  entity_increment_hunger(human);
  entity_increment_thirst(human);
  entity_increment_tiredness(human);

  CU_ASSERT_EQUAL(entity_get_hunger(human), 43);
  CU_ASSERT_EQUAL(entity_get_thirst(human), 6);
  CU_ASSERT_EQUAL(entity_get_tiredness(human), 99);

  entity_free(human);
}

void entity_lifepoints_test(void) {
  Entity *monster = entity_build(15, INHUMAN, "Orc", 20, 20);
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

void entity_mental_health_test(void) {
  EntityBuilder *builder = entity_builder_new();
  Entity *sane = builder->with_type(builder, ANIMAL)->with_name(builder, "E1")->with_mental_health(builder, 500)->build(builder, true);

  CU_ASSERT_EQUAL(entity_get_mental_health(sane), 500);
  CU_ASSERT_TRUE(entity_is_sane(sane));

  entity_mental_hurt(sane, 450);
  CU_ASSERT_EQUAL(entity_get_mental_health(sane), 50);
  CU_ASSERT_TRUE(entity_is_sane(sane));

  entity_mental_hurt(sane, 80);
  CU_ASSERT_EQUAL(entity_get_mental_health(sane), 0);
  CU_ASSERT_FALSE(entity_is_sane(sane));
  CU_ASSERT_TRUE(entity_is_crazy(sane));

  entity_mental_heal(sane, 380);
  CU_ASSERT_EQUAL(entity_get_mental_health(sane), 380);
  CU_ASSERT_FALSE(entity_is_crazy(sane));

  entity_mental_heal(sane, 200);
  CU_ASSERT_EQUAL(entity_get_mental_health(sane), 500);
  CU_ASSERT_FALSE(entity_is_crazy(sane));

  entity_free(sane);
}

void entity_resurrect_test(void) {
  Entity *human = entity_build(15, HUMAN, "human", 0, 0);
  Entity *inhuman = entity_build(15, INHUMAN, "inhuman", 0, 0);
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
  Entity *dog = entity_build(30, ANIMAL, "Bounty", 46, -8);
  CU_ASSERT_TRUE(entity_can_move(dog));

  entity_move(dog, -4, 20);

  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(dog)), 42);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(dog)), 12);

  Entity *tree = entity_build(1, TREE, "Ent", 20, 1);
  CU_ASSERT_FALSE(entity_can_move(tree));
  entity_move(tree, 10, 14);
  CU_ASSERT_EQUAL(point_get_x(entity_get_coords(tree)), 20);
  CU_ASSERT_EQUAL(point_get_y(entity_get_coords(tree)), 1);

  Entity *mountain = entity_build(1, MOUNTAIN, "Fuji", 10, 14);
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

bool filter_weapons(Item const *item) {
  return strncmp(item_get_name(item), "Weapon", 6) == 0;
}

bool filter_tools(Item const *item) {
  return item_get_type(item) == TOOL;
}

bool filter_none(Item const *) {
  return false;
}

void entity_inventory_test(void) {
  Entity *entity = entity_build(20, HUMAN, "An human", 0, 0);
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

  Entity *empty_inventory = entity_build(10, TREE, "A tree", 0, 0);
  entity_inventory_remove_item(empty_inventory, "Not existing");
  CU_ASSERT_EQUAL(entity_inventory_count(empty_inventory), 0);

  free(nothing);
  free(tools);
  free(weapons);
  entity_free(empty_inventory);
  entity_free(entity);
}

void entity_serialization_test() {
  const char *filename = "./entity_serialization.bin";

  Entity *entity = entity_build(30, HUMAN, "Some random name", 42, 23);
  entity_inventory_add_item(entity, tool_new("Pickaxe", 30, 15, 2, 14));
  entity_inventory_add_item(entity, armor_new("An armor", 30, 16, 50, 30, 4));
  entity_inventory_add_item(entity, item_new(FORAGE, "A fruit", 10, 30));

  entity_perks_add(entity, perk_new(PT_ITEMS_STATS, "Mechanic"));
  entity_perks_add(entity, perk_new(PT_ENTITY_STATS, "NightVision"));

  // Simulate something in the engine
  entity_hurt(entity, 12);
  entity_move(entity, 3, -4);

  msgpack_sbuffer buffer;
  msgpack_sbuffer_init(&buffer);

  entity_serialize(entity, &buffer);
  CU_ASSERT_PTR_NOT_NULL(buffer.data);
  CU_ASSERT_TRUE(buffer.size > 0);

  // Write to file
  int file_fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  write(file_fd, buffer.data, buffer.size);
  close(file_fd);

  // Load content of file into a newly created buffer;
  char  *bin_buffer;
  FILE  *bin_file = fopen(filename, "rb");
  size_t bin_length = file_size(bin_file);
  bin_buffer = malloc(bin_length);

  fread(bin_buffer, bin_length, sizeof(char), bin_file);
  fclose(bin_file);

  unlink(filename);

  CU_ASSERT_TRUE(strings_equal(buffer.data, bin_buffer));

  // Start the deserialization process
  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, 0);
  msgpack_unpacker_reserve_buffer(&unpacker, bin_length);
  memcpy(msgpack_unpacker_buffer(&unpacker), bin_buffer, bin_length);
  msgpack_unpacker_buffer_consumed(&unpacker, bin_length);

  msgpack_unpacked result;
  msgpack_unpacked_init(&result);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);

  CU_ASSERT_EQUAL(result.data.type, MSGPACK_OBJECT_MAP);
  CU_ASSERT_EQUAL(result.data.via.map.size, 17);

#define serde_map_assert_with_value(type, ctype, field, expected_value)                            \
  {                                                                                                \
    serde_map_assert(&result.data.via.map, MSGPACK_OBJECT_##type, #field);                         \
    ctype from_map = *(ctype *)serde_map_get(&result.data.via.map, MSGPACK_OBJECT_##type, #field); \
    CU_ASSERT_EQUAL(from_map, expected_value);                                                     \
  }

  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, lp, 30 - 12);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, starting_lp, 30);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, mental_health, 30);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, starting_mental_health, 30);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, hunger, 0);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, thirst, 0);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, tiredness, 0);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, xp, 0);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, current_level, 0);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, hearing_distance, 10);
  serde_map_assert_with_value(POSITIVE_INTEGER, uint32_t, seeing_distance, 10);
  serde_map_assert_with_value(POSITIVE_INTEGER, EntityType, type, HUMAN);

  serde_map_assert(&result.data.via.map, MSGPACK_OBJECT_STR, "name");
  serde_map_assert(&result.data.via.map, MSGPACK_OBJECT_ARRAY, "coords");
  serde_map_assert(&result.data.via.map, MSGPACK_OBJECT_ARRAY, "inventory");
  serde_map_assert(&result.data.via.map, MSGPACK_OBJECT_ARRAY, "perks");

  msgpack_object_str const *name_str = serde_map_get(&result.data.via.map, MSGPACK_OBJECT_STR, "name");
  serde_assert_str(name_str, "Some random name");

  msgpack_object_array const *coords = serde_map_get(&result.data.via.map, MSGPACK_OBJECT_ARRAY, "coords");
  CU_ASSERT_EQUAL(coords->size, 2);

  CU_ASSERT_EQUAL(coords->ptr[0].via.u64, 45);
  CU_ASSERT_EQUAL(coords->ptr[1].via.u64, 19);

  msgpack_object_array const *inventory = serde_map_get(&result.data.via.map, MSGPACK_OBJECT_ARRAY, "inventory");
  CU_ASSERT_EQUAL(inventory->size, 3);

  msgpack_object_array const *perks = serde_map_get(&result.data.via.map, MSGPACK_OBJECT_ARRAY, "perks");
  CU_ASSERT_EQUAL(perks->size, 2);

  msgpack_unpacker_destroy(&unpacker);
  msgpack_unpacked_destroy(&result);
  entity_free(entity);
}

void entity_deserialize_test(void) {
  msgpack_sbuffer buffer;
  msgpack_sbuffer_init(&buffer);
  Entity *entity = entity_build(10, ANIMAL, "Bounty the cat", 10, 11);
  entity_inventory_add_item(entity, armor_new("Hairy armor", 10, 10, 0, 0, 15));
  entity_inventory_add_item(entity, tool_new("Lighter", 1, 1, 1, 10));
  entity_inventory_add_item(entity, item_new(FORAGE, "An apple", 1, 1));

  entity_perks_add(entity, perk_new(PT_ENVIRONMENT, "AlwaysLit"));
  entity_perks_add(entity, perk_new(PT_ENTITY_STATS, "NeverTired"));

  entity_hurt(entity, 3);
  entity_serialize(entity, &buffer);

  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, 0);
  msgpack_unpacker_reserve_buffer(&unpacker, buffer.size);
  memcpy(msgpack_unpacker_buffer(&unpacker), buffer.data, buffer.size);
  msgpack_unpacker_buffer_consumed(&unpacker, buffer.size);

  msgpack_unpacked result;
  msgpack_unpacked_init(&result);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);

  Entity *rebuilt = entity_deserialize(&result.data.via.map);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(entity), entity_get_name(rebuilt)));
  CU_ASSERT_TRUE(points_equal(entity_get_coords(entity), entity_get_coords(rebuilt)));

#define CU_ASSERT_ENTITY_PROP(prop) CU_ASSERT_EQUAL(entity_##prop(entity), entity_##prop(rebuilt));

  CU_ASSERT_ENTITY_PROP(get_life_points);
  CU_ASSERT_ENTITY_PROP(get_starting_life_points);
  CU_ASSERT_ENTITY_PROP(get_mental_health);
  CU_ASSERT_ENTITY_PROP(get_starting_mental_health);
  CU_ASSERT_ENTITY_PROP(get_hunger);
  CU_ASSERT_ENTITY_PROP(get_thirst);
  CU_ASSERT_ENTITY_PROP(get_tiredness);
  CU_ASSERT_ENTITY_PROP(get_xp);
  CU_ASSERT_ENTITY_PROP(get_current_level);
  CU_ASSERT_ENTITY_PROP(get_hearing_distance);
  CU_ASSERT_ENTITY_PROP(get_seeing_distance);
  CU_ASSERT_ENTITY_PROP(get_entity_type);
  CU_ASSERT_ENTITY_PROP(inventory_count);

  Item **entity_inventory = entity_inventory_get(entity);
  Item **rebuilt_inventory = entity_inventory_get(rebuilt);

  for (uint i = 0; i < entity_inventory_count(rebuilt); i++) {
    // We're not checking the whole item since that is already done in the item_test
    CU_ASSERT_EQUAL(item_get_type(entity_inventory[i]), item_get_type(rebuilt_inventory[i]));
  }

  CU_ASSERT_EQUAL(entity_perks_count(entity), entity_perks_count(rebuilt));

  Perk const *original_perk = entity_perks_get(entity, "AlwaysLit");
  Perk const *rebuilt_perk = entity_perks_get(rebuilt, "AlwaysLit");
  CU_ASSERT_PTR_NOT_NULL(original_perk);
  CU_ASSERT_PTR_NOT_NULL(rebuilt_perk);

  msgpack_unpacker_destroy(&unpacker);
  msgpack_sbuffer_destroy(&buffer);
  entity_free(entity);
  entity_free(rebuilt);
}

void entity_builder_test(void) {
  EntityBuilder *builder = entity_builder_new();
  const char    *fmt = "%s #%d";

  uint32_t starting_xp = 100;

  // Default values for most of the entities we're going to create
  builder->with_seeing_distance(builder, 1)
    ->with_type(builder, ANIMAL)
    ->with_xp(builder, starting_xp)
    ->with_life_points(builder, 5)
    ->with_mental_health(builder, 10)
    ->with_hearing_distance(builder, 100)
    ->with_level(builder, 1);

  char *name = calloc(1024, sizeof(char));

  Entity **results = calloc(10, sizeof(Entity *));
  for (uint i = 0; i < 10; i++) {
    sprintf(name, fmt, "A bat", i);
    results[i] = builder->with_name(builder, name)->with_xp(builder, builder->xp + 10)->build(builder, false);
    memset(name, 0, strlen(name));
  }

  // Verifications
  for (uint i = 0; i < 10; i++) {
    Entity const *entity = results[i];

    sprintf(name, fmt, "A bat", i);
    CU_ASSERT_EQUAL(entity_get_xp(entity), starting_xp + 10);
    CU_ASSERT_EQUAL(entity_get_life_points(entity), 5);
    CU_ASSERT_EQUAL(entity_get_mental_health(entity), 10);
    CU_ASSERT_EQUAL(entity_get_hearing_distance(entity), 100);
    CU_ASSERT_EQUAL(entity_get_seeing_distance(entity), 1);
    CU_ASSERT_EQUAL(entity_get_entity_type(entity), ANIMAL);
    CU_ASSERT_EQUAL(entity_get_current_level(entity), 1);
    CU_ASSERT_TRUE(strings_equal(entity_get_name(entity), name));

    starting_xp += 10;

    memset(name, 0, strlen(name));
  }

  entity_builder_free(builder);
  for (uint i = 0; i < 10; i++) {
    entity_free(results[i]);
  }

  free(name);
  free(results);
}

bool perk_filter(Perk const *perk) {
  return strings_equal(perk_get_name(perk), "MentalAugmented") || strings_equal(perk_get_name(perk), "NightVision");
}

void entity_perks_test(void) {
  EntityBuilder *builder = entity_builder_new();

  Entity *entity = builder->with_name(builder, "Perked Entity")->with_xp(builder, 300)->build(builder, true);

  CU_ASSERT_EQUAL(entity_perks_count(entity), 0);

  const char *stuff[] = {
    "MentalAugmented",
    "LifeAugmented",
    "NightVision",
    "Robustness",
  };

  for (uint8_t i = 0; i < 4; i++) {
    entity_perks_add(entity, perk_new(PT_ITEMS_STATS, stuff[i]));
  }

  CU_ASSERT_EQUAL(entity_perks_count(entity), 4);
  CU_ASSERT_TRUE(entity_perks_has_perk(entity, "NightVision"));
  CU_ASSERT_TRUE(entity_perks_has_perk(entity, "LifeAugmented"));
  CU_ASSERT_FALSE(entity_perks_has_perk(entity, "NonExisting"));

  Perk const *night_vision = entity_perks_get(entity, "NightVision");
  CU_ASSERT_PTR_NOT_NULL(night_vision);
  CU_ASSERT_EQUAL(perk_get_perk_type(night_vision), PT_ITEMS_STATS);

  CU_ASSERT_PTR_NULL(entity_perks_get(entity, "NotExisting"));

  size_t filtered_list_size;
  Perk **filtered = entity_perks_filter(entity, &perk_filter, &filtered_list_size);

  CU_ASSERT_EQUAL(filtered_list_size, 2);
  CU_ASSERT_TRUE(strings_equal(perk_get_name(filtered[0]), "MentalAugmented"));
  CU_ASSERT_TRUE(strings_equal(perk_get_name(filtered[1]), "NightVision"));

  // Make sure we can walk the list either using nullptr termination or in a classic way
  for (size_t i = 0; i < filtered_list_size; i++) {
    CU_ASSERT_PTR_NOT_NULL(filtered[i]);
  }

  size_t      count = 0;
  Perk const *current_item = filtered[count];
  while (current_item != nullptr) {
    bool ma_bool = strings_equal(perk_get_name(current_item), "MentalAugmented");
    bool nv_bool = strings_equal(perk_get_name(current_item), "NightVision");

    CU_ASSERT_TRUE(ma_bool || nv_bool);
    current_item = filtered[++count];
  }

  CU_ASSERT_EQUAL(count, 2);
  free(filtered);

  entity_free(entity);
}

void entity_equipment_test(void) {
  Entity *entity = entity_build(10, HUMAN, "Human", 0, 0);
  entity_inventory_add_item(entity, weapon_new("One handed", 10, 0, 1, 10, 10));
  entity_inventory_add_item(entity, weapon_new("One handed bis", 10, 0, 1, 10, 10));
  entity_inventory_add_item(entity, weapon_new("Two handed", 10, 0, 2, 10, 10));
  entity_inventory_add_item(entity, armor_new("Helmet", 1, 0, 0, 10, 2));
  entity_inventory_add_item(entity, armor_new("Necklace", 1, 0, 2, 10, 3));
  entity_inventory_add_item(entity, armor_new("T-Shirt", 1, 0, 2, 10, 3));
  entity_inventory_add_item(entity, armor_new("Jeans", 3, 0, 1, 10, 0));
  entity_inventory_add_item(entity, armor_new("Left shoe", 2, 0, 1, 10, 4));
  entity_inventory_add_item(entity, armor_new("Right shoe", 2, 0, 1, 10, 4));
  entity_inventory_add_item(entity, tool_new("Lighter", 0, 0, 1, 10));

  CU_ASSERT_EQUAL(entity_inventory_count(entity), 10);

  // One handed and two handed weapons
  entity_equipment_set_left_hand(entity, "One handed");
  CU_ASSERT_PTR_NOT_NULL(entity_equipment_get_left_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 9);

  entity_equipment_set_right_hand(entity, "One handed bis");
  CU_ASSERT_PTR_NOT_NULL(entity_equipment_get_right_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 8);

  entity_equipment_unset_right_hand(entity);
  entity_equipment_unset_left_hand(entity);
  CU_ASSERT_PTR_NULL(entity_equipment_get_left_hand(entity));
  CU_ASSERT_PTR_NULL(entity_equipment_get_right_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 10);

  entity_equipment_set_left_hand(entity, "Two handed");
  CU_ASSERT_PTR_NOT_NULL(entity_equipment_get_left_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 9);

  // Cannot equip to right hand since we already have two handed in left hand
  entity_equipment_set_right_hand(entity, "One handed");
  CU_ASSERT_PTR_NULL(entity_equipment_get_right_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 9);

  entity_equipment_unset_left_hand(entity);
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 10);

  // Trying to equip 2-handed in right hand should equip to left hand instead
  entity_equipment_set_right_hand(entity, "Two handed");
  CU_ASSERT_PTR_NULL(entity_equipment_get_right_hand(entity));
  CU_ASSERT_PTR_NOT_NULL(entity_equipment_get_left_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 9);

  entity_equipment_unset_left_hand(entity);
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 10);

  // Trying to equip an armor in hand should result in a failure
  entity_equipment_set_left_hand(entity, "Necklace");
  CU_ASSERT_PTR_NULL(entity_equipment_get_left_hand(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 10);

  // Trying to wear a weapon should result in a failure
  entity_equipment_set_head(entity, "Two handed");
  CU_ASSERT_PTR_NULL(entity_equipment_get_head(entity));
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 10);

  // Wear full armor
  entity_equipment_set_head(entity, "Helmet");
  entity_equipment_set_neck(entity, "Necklace");
  entity_equipment_set_torso(entity, "T-Shirt");
  entity_equipment_set_legs(entity, "Jeans");
  entity_equipment_set_left_foot(entity, "Left shoe");
  entity_equipment_set_right_foot(entity, "Right shoe");
  CU_ASSERT_EQUAL(entity_inventory_count(entity), 4);

  entity_free(entity);
}

void entity_test_suite() {
  CU_pSuite suite = CU_add_suite("Entity Tests", nullptr, nullptr);
  CU_add_test(suite, "Create a basic entity", &entity_creation_test);
  CU_add_test(suite, "Manipulate life points", &entity_lifepoints_test);
  CU_add_test(suite, "Manipulate mental health", &entity_mental_health_test);
  CU_add_test(suite, "Move entities", &entity_move_test);
  CU_add_test(suite, "Resurrect entities", &entity_resurrect_test);
  CU_add_test(suite, "Serialization", &entity_serialization_test);
  CU_add_test(suite, "Deserialization", &entity_deserialize_test);
  CU_add_test(suite, "Inventory manipulation", &entity_inventory_test);
  CU_add_test(suite, "Equipment manipulation", &entity_equipment_test);
  CU_add_test(suite, "Perks manipulation", &entity_perks_test);
  CU_add_test(suite, "Entity Builder", &entity_builder_test);
}

