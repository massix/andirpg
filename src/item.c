#include "item.h"
#include "logger.h"
#include "point.h"
#include <assert.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct Item {
  ItemType _type;
  char    *_name;
  uint32_t _weight;
  uint32_t _value;
  void    *_properties;
  Point   *_coords;
};

struct WeaponProperties {
  uint8_t  _hands; // 1 or 2 hands weapon
  uint32_t _attack_power;
  uint8_t  _life_points;
};

struct ToolProperties {
  uint8_t _hands;
  uint8_t _life_points;
};

struct ArmorProperties {
  uint32_t _defense_value;
  uint8_t  _life_points;
  uint8_t  _armor_class;
};

// Generic Item, used as a placeholder
Item *item_new(ItemType item_type, const char *name, uint32_t weight, uint32_t value) {
  Item *ret = calloc(1, sizeof(Item));
  LOG_INFO("Creating new item '%s'", name);

  ret->_type = item_type;
  ret->_name = strdup(name);
  ret->_weight = weight;
  ret->_value = value;
  ret->_properties = nullptr;
  ret->_coords = nullptr;

  return ret;
}

#define WRITE_MAP_KEY(packer, key) msgpack_pack_str_with_body(&(packer), #key, strlen(#key))

// Forward declarations
void armor_serialize(Item *, msgpack_packer *);
void tool_serialize(Item *, msgpack_packer *);
void forage_serialize(Item *, msgpack_packer *);
void weapon_serialize(Item *, msgpack_packer *);
void gem_serialize(Item *, msgpack_packer *);

void item_serialize(Item *item, msgpack_sbuffer *buffer) {
  LOG_INFO("Packing generic item", 0);

  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);
  msgpack_pack_map(&packer, 6);

  WRITE_MAP_KEY(packer, type);
  msgpack_pack_uint8(&packer, item->_type);

  WRITE_MAP_KEY(packer, name);
  msgpack_pack_str_with_body(&packer, item->_name, strlen(item->_name));

  WRITE_MAP_KEY(packer, weight);
  msgpack_pack_uint32(&packer, item->_weight);

  WRITE_MAP_KEY(packer, value);
  msgpack_pack_uint32(&packer, item->_value);

  WRITE_MAP_KEY(packer, coords);
  if (item_has_coords(item)) {
    msgpack_pack_array(&packer, 2);
    msgpack_pack_uint32(&packer, point_get_x(item->_coords));
    msgpack_pack_uint32(&packer, point_get_y(item->_coords));
  } else {
    msgpack_pack_array(&packer, 0);
  }

  WRITE_MAP_KEY(packer, properties);
  switch (item->_type) {
    case ARMOR:
      armor_serialize(item, &packer);
      break;
    case TOOL:
      tool_serialize(item, &packer);
      break;
    case WEAPON:
      weapon_serialize(item, &packer);
      break;
    case FORAGE:
      forage_serialize(item, &packer);
      break;
    case GEM:
      gem_serialize(item, &packer);
      break;
    default:
      msgpack_pack_nil(&packer);
      break;
  }

  msgpack_pack_nil(&packer);
}

bool strnequal(const char *lhs, const char *rhs, size_t size) {
  return strncmp(lhs, rhs, size) == 0;
}

msgpack_object_kv *find_key(msgpack_object_map *map, const char *key) {
  msgpack_object_kv *return_value = nullptr;
  for (uint32_t i = 0; i < map->size; i++) {
    if (strnequal(map->ptr[i].key.via.str.ptr, key, map->ptr[i].key.via.str.size)) {
      return_value = &map->ptr[i];
    }
  }

  return return_value;
}

#define ASSERT_MAP_HEADER(pointer, val) assert(strnequal((pointer).key.via.str.ptr, val, (pointer).key.via.str.size));

void item_deserialize_check_map(msgpack_object_map *msgpack_map) {
  LOG_INFO("Item map validation", 0);

  // Check that the received object is indeed an item, it must contain
  // exactly 6 fields and have exactly the fields we want
  assert(msgpack_map->size == 6);
  for (uint32_t i = 0; i < 6; i++) {
    assert(msgpack_map->ptr[i].key.type == MSGPACK_OBJECT_STR);
  }

  ASSERT_MAP_HEADER(msgpack_map->ptr[0], "type");
  ASSERT_MAP_HEADER(msgpack_map->ptr[1], "name");
  ASSERT_MAP_HEADER(msgpack_map->ptr[2], "weight");
  ASSERT_MAP_HEADER(msgpack_map->ptr[3], "value");
  ASSERT_MAP_HEADER(msgpack_map->ptr[4], "coords");
  ASSERT_MAP_HEADER(msgpack_map->ptr[5], "properties");
}

void armor_deserialize_check_map(msgpack_object_map *msgmap) {
  LOG_INFO("Armor map validation", 0);

  assert(msgmap->size == 3);
  ASSERT_MAP_HEADER(msgmap->ptr[0], "defense_value");
  ASSERT_MAP_HEADER(msgmap->ptr[1], "armor_class");
  ASSERT_MAP_HEADER(msgmap->ptr[2], "life_points");
}

ArmorProperties *armor_deserialize(msgpack_object_map *msgmap) {
  LOG_INFO("Deserializing armor properties", 0);
  armor_deserialize_check_map(msgmap);

  ArmorProperties *props = calloc(1, sizeof(ArmorProperties));
  props->_armor_class = find_key(msgmap, "armor_class")->val.via.u64;
  props->_life_points = find_key(msgmap, "life_points")->val.via.u64;
  props->_defense_value = find_key(msgmap, "defense_value")->val.via.u64;

  return props;
}

void tool_deserialize_check_map(msgpack_object_map *msgmap) {
  LOG_INFO("Tool map validation", 0);

  assert(msgmap->size == 2);
  ASSERT_MAP_HEADER(msgmap->ptr[0], "hands");
  ASSERT_MAP_HEADER(msgmap->ptr[1], "life_points");
}

ToolProperties *tool_deserialize(msgpack_object_map *msgmap) {
  LOG_INFO("Deserializing tool properties", 0);

  ToolProperties *props = calloc(1, sizeof(ToolProperties));
  props->_hands = find_key(msgmap, "hands")->val.via.u64;
  props->_life_points = find_key(msgmap, "life_points")->val.via.u64;

  return props;
}

void weapon_deserialize_check_map(msgpack_object_map *msgmap) {
  LOG_INFO("Weapon map validation", 0);

  assert(msgmap->size == 3);
  ASSERT_MAP_HEADER(msgmap->ptr[0], "attack_power");
  ASSERT_MAP_HEADER(msgmap->ptr[1], "life_points");
  ASSERT_MAP_HEADER(msgmap->ptr[2], "hands");
}

WeaponProperties *weapon_deserialize(msgpack_object_map *msgmap) {
  LOG_INFO("Deserializing weapon properties", 0);
  weapon_deserialize_check_map(msgmap);

  WeaponProperties *props = calloc(1, sizeof(ArmorProperties));
  props->_attack_power = find_key(msgmap, "attack_power")->val.via.u64;
  props->_life_points = find_key(msgmap, "life_points")->val.via.u64;
  props->_hands = find_key(msgmap, "hands")->val.via.u64;

  return props;
}

Item *item_deserialize(msgpack_object_map *msgpack_map) {
  LOG_INFO("Starting deserialization of item", 0);
  item_deserialize_check_map(msgpack_map);

  ItemType item_type = (ItemType)find_key(msgpack_map, "type")->val.via.u64;

  msgpack_object_kv *name_kv = find_key(msgpack_map, "name");
  char              *name = malloc(name_kv->val.via.str.size);
  memcpy(name, name_kv->val.via.str.ptr, name_kv->val.via.str.size);

  Item *final_item = item_new(item_type, name, find_key(msgpack_map, "weight")->val.via.u64, find_key(msgpack_map, "value")->val.via.u64);

  msgpack_object_map *props = &find_key(msgpack_map, "properties")->val.via.map;

  // TODO: specialized deserializations here
  switch (item_type) {
    case ARMOR:
      final_item->_properties = armor_deserialize(props);
      break;
    case TOOL:
      final_item->_properties = tool_deserialize(props);
      break;
    case WEAPON:
      final_item->_properties = weapon_deserialize(props);
      break;
    case FORAGE:
    case GEM:
      final_item->_properties = nullptr;
      break;
  }

  free(name);
  return final_item;
}

// Specialized serialization
void armor_serialize(Item *item, msgpack_packer *packer) {
  LOG_INFO("Packing armor", 0);
  ArmorProperties *properties = (ArmorProperties *)item->_properties;
  msgpack_pack_map(packer, 3);

  WRITE_MAP_KEY(*packer, defense_value);
  msgpack_pack_uint32(packer, properties->_defense_value);

  WRITE_MAP_KEY(*packer, armor_class);
  msgpack_pack_uint8(packer, properties->_armor_class);

  WRITE_MAP_KEY(*packer, life_points);
  msgpack_pack_uint8(packer, properties->_life_points);
}

// Specialized serialization
void tool_serialize(Item *item, msgpack_packer *packer) {
  LOG_INFO("Packing tool", 0);
  ToolProperties *properties = (ToolProperties *)item->_properties;
  msgpack_pack_map(packer, 2);

  WRITE_MAP_KEY(*packer, hands);
  msgpack_pack_uint8(packer, properties->_hands);

  WRITE_MAP_KEY(*packer, life_points);
  msgpack_pack_uint8(packer, properties->_life_points);
}

// Specialized serialization
void weapon_serialize(Item *item, msgpack_packer *packer) {
  LOG_INFO("Packing weapon", 0);
  WeaponProperties *properties = (WeaponProperties *)item->_properties;
  msgpack_pack_map(packer, 3);

  WRITE_MAP_KEY(*packer, attack_power);
  msgpack_pack_uint32(packer, properties->_attack_power);

  WRITE_MAP_KEY(*packer, life_points);
  msgpack_pack_uint8(packer, properties->_life_points);

  WRITE_MAP_KEY(*packer, hands);
  msgpack_pack_uint8(packer, properties->_hands);
}

// Specialized serialization
void forage_serialize(Item *item, msgpack_packer *packer) {
  LOG_WARNING("Forage items not implemented yet, cannot pack it", 0);
  msgpack_pack_nil(packer);
}

// Specialized serialization
void gem_serialize(Item *item, msgpack_packer *packer) {
  LOG_WARNING("Gem items not implemented yet, cannot pack it", 0);
  msgpack_pack_nil(packer);
}

Item *item_clone(Item *origin) {
  Item *ret = calloc(1, sizeof(Item));
  LOG_INFO("Cloning item '%s'", item_get_name(origin));

  ret->_type = origin->_type;
  ret->_name = strdup(origin->_name);
  ret->_weight = origin->_weight;
  ret->_value = origin->_value;

  if (item_has_properties(origin)) {
    if (origin->_type == WEAPON) {
      LOG_DEBUG("Item is a weapon", 0);
      ret->_properties = calloc(1, sizeof(WeaponProperties));
      WeaponProperties *origin_properties = (WeaponProperties *)origin->_properties;
      WeaponProperties *target_properties = (WeaponProperties *)ret->_properties;

      target_properties->_hands = origin_properties->_hands;
      target_properties->_life_points = origin_properties->_life_points;
      target_properties->_attack_power = origin_properties->_attack_power;
    } else if (origin->_type == TOOL) {
      LOG_DEBUG("Item is a tool", 0);
      ret->_properties = calloc(1, sizeof(ToolProperties));
      ToolProperties *origin_properties = (ToolProperties *)origin->_properties;
      ToolProperties *target_properties = (ToolProperties *)ret->_properties;

      target_properties->_life_points = origin_properties->_life_points;
      target_properties->_hands = origin_properties->_hands;
    } else if (origin->_type == ARMOR) {
      LOG_DEBUG("Item is an armor", 0);
      ret->_properties = calloc(1, sizeof(ArmorProperties));
      ArmorProperties *origin_properties = (ArmorProperties *)origin->_properties;
      ArmorProperties *target_properties = (ArmorProperties *)ret->_properties;

      target_properties->_life_points = origin_properties->_life_points;
      target_properties->_armor_class = origin_properties->_armor_class;
      target_properties->_defense_value = origin_properties->_defense_value;
    }
  }

  if (item_has_coords(origin)) {
    LOG_DEBUG("Item has coordinates", 0);
    ret->_coords = point_new(point_get_x(item_get_coords(origin)), point_get_y(item_get_coords(origin)));
  }

  return ret;
}

Item *weapon_new(const char *name, uint32_t weight, uint32_t value, uint8_t hands, uint32_t attack_power, uint8_t life_points) {
  Item *ret = item_new(WEAPON, name, weight, value);
  LOG_DEBUG("Creating new weapon '%s'", name);

  ret->_properties = calloc(1, sizeof(WeaponProperties));

  WeaponProperties *properties = ret->_properties;

  properties->_hands = hands;
  properties->_life_points = life_points;
  properties->_attack_power = attack_power;

  return ret;
}

Item *tool_new(const char *name, uint32_t weight, uint32_t value, uint8_t hands, uint8_t life_points) {
  Item *ret = item_new(TOOL, name, weight, value);
  LOG_DEBUG("Creating new tool '%s'", name);

  ret->_properties = calloc(1, sizeof(ToolProperties));

  ToolProperties *properties = ret->_properties;

  properties->_hands = hands;
  properties->_life_points = life_points;

  return ret;
}

Item *armor_new(const char *name, uint32_t weight, uint32_t value, uint32_t defense_value, uint8_t life_points, uint8_t armor_class) {
  Item *ret = item_new(ARMOR, name, weight, value);
  LOG_DEBUG("Creating new armor '%s'", name);

  ret->_properties = calloc(1, sizeof(ArmorProperties));

  ArmorProperties *properties = ret->_properties;

  properties->_defense_value = defense_value;
  properties->_life_points = life_points;
  properties->_armor_class = armor_class;

  return ret;
}

inline ItemType item_get_type(Item *item) {
  return item->_type;
}

inline const char *item_get_name(Item *item) {
  return item->_name;
}

inline uint32_t item_get_weight(Item *item) {
  return item->_weight;
}

inline uint32_t item_get_value(Item *item) {
  return item->_value;
}

inline bool item_has_properties(Item *item) {
  return item->_properties != nullptr;
}

inline void *item_get_properties(Item *item) {
  return item->_properties;
}

inline uint8_t weapon_get_hands(WeaponProperties *weapon) {
  return weapon->_hands;
}

inline uint32_t weapon_get_attack_power(WeaponProperties *weapon) {
  return weapon->_attack_power;
}

inline uint8_t weapon_get_life_points(WeaponProperties *weapon) {
  return weapon->_life_points;
}

inline uint8_t tool_get_hands(ToolProperties *tool) {
  return tool->_hands;
}

inline uint8_t tool_get_life_points(ToolProperties *tool) {
  return tool->_life_points;
}

inline uint32_t armor_get_defense_value(ArmorProperties *armor) {
  return armor->_defense_value;
}

inline uint8_t armor_get_life_points(ArmorProperties *armor) {
  return armor->_life_points;
}

inline uint8_t armor_get_armor_class(ArmorProperties *armor) {
  return armor->_armor_class;
}

inline bool item_has_coords(Item *item) {
  return item->_coords != nullptr;
}

inline Point *item_get_coords(Item *item) {
  return item->_coords;
}

void item_set_coords(Item *item, uint32_t x, uint32_t y) {
  LOG_INFO("Setting coords for item '%s'", item_get_name(item));
  item_clear_coords(item);

  item->_coords = point_new(x, y);
}

void item_clear_coords(Item *item) {
  if (item_has_coords(item)) {
    LOG_INFO("Cleaning coords for item '%s'", item_get_name(item));
    point_free(item->_coords);
  }

  item->_coords = nullptr;
}

void item_free(Item *item) {
  if (item_has_properties(item)) {
    free(item->_properties);
  }

  item_clear_coords(item);

  free(item->_name);
  free(item);
}

