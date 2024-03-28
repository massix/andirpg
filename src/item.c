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
#include "logger.h"
#include "point.h"
#include "serde.h"
#include "utils.h"
#include <assert.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Private methods
void armor_serialize(Item const *, msgpack_packer *);
void tool_serialize(Item const *, msgpack_packer *);
void forage_serialize(Item const *, msgpack_packer *);
void weapon_serialize(Item const *, msgpack_packer *);
void gem_serialize(Item const *, msgpack_packer *);
void item_deserialize_check_map(msgpack_object_map const *msgpack_map);
void armor_deserialize_check_map(msgpack_object_map const *msgmap);
void tool_deserialize_check_map(msgpack_object_map const *msgmap);
void weapon_deserialize_check_map(msgpack_object_map const *msgmap);

ArmorProperties  *armor_deserialize(msgpack_object_map const *msgmap);
ToolProperties   *tool_deserialize(msgpack_object_map const *msgmap);
WeaponProperties *weapon_deserialize(msgpack_object_map const *msgmap);

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

Item *item_clone(Item const *origin) {
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
      WeaponProperties const *origin_properties = (WeaponProperties *)origin->_properties;
      WeaponProperties       *target_properties = (WeaponProperties *)ret->_properties;

      target_properties->_hands = origin_properties->_hands;
      target_properties->_life_points = origin_properties->_life_points;
      target_properties->_attack_power = origin_properties->_attack_power;
    } else if (origin->_type == TOOL) {
      LOG_DEBUG("Item is a tool", 0);
      ret->_properties = calloc(1, sizeof(ToolProperties));
      ToolProperties const *origin_properties = (ToolProperties *)origin->_properties;
      ToolProperties       *target_properties = (ToolProperties *)ret->_properties;

      target_properties->_life_points = origin_properties->_life_points;
      target_properties->_hands = origin_properties->_hands;
    } else if (origin->_type == ARMOR) {
      LOG_DEBUG("Item is an armor", 0);
      ret->_properties = calloc(1, sizeof(ArmorProperties));
      ArmorProperties const *origin_properties = (ArmorProperties *)origin->_properties;
      ArmorProperties       *target_properties = (ArmorProperties *)ret->_properties;

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

Item *item_deserialize(msgpack_object_map const *msgpack_map) {
  LOG_INFO("Starting deserialization of item", 0);
  item_deserialize_check_map(msgpack_map);

  ItemType item_type = *(ItemType *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "type");

  msgpack_object_str const *name_kv = (msgpack_object_str *)serde_map_get(msgpack_map, MSGPACK_OBJECT_STR, "name");

  char *name = malloc(name_kv->size);
  memcpy(name, name_kv->ptr, name_kv->size);

  uint32_t const *weight = (uint32_t *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "weight");
  uint32_t const *value = (uint32_t *)serde_map_get(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "value");

  Item *final_item = item_new(item_type, name, *weight, *value);

  msgpack_object_map const *props = serde_map_get(msgpack_map, MSGPACK_OBJECT_MAP, "properties");

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

void item_serialize(Item const *item, msgpack_sbuffer *buffer) {
  LOG_INFO("Packing generic item", 0);

  msgpack_packer packer;
  msgpack_packer_init(&packer, buffer, &msgpack_sbuffer_write);
  msgpack_pack_map(&packer, 6);

  serde_pack_str(&packer, "type");
  msgpack_pack_uint8(&packer, item->_type);

  serde_pack_str(&packer, "name");
  serde_pack_str(&packer, item->_name);

  serde_pack_str(&packer, "weight");
  msgpack_pack_uint32(&packer, item->_weight);

  serde_pack_str(&packer, "value");
  msgpack_pack_uint32(&packer, item->_value);

  serde_pack_str(&packer, "coords");
  if (item_has_coords(item)) {
    msgpack_pack_array(&packer, 2);
    msgpack_pack_uint32(&packer, point_get_x(item->_coords));
    msgpack_pack_uint32(&packer, point_get_y(item->_coords));
  } else {
    msgpack_pack_array(&packer, 0);
  }

  serde_pack_str(&packer, "properties");
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
}

void item_free(Item *item) {
  if (item_has_properties(item)) {
    free(item->_properties);
  }

  item_clear_coords(item);

  free(item->_name);
  free(item);
}

inline bool item_has_properties(Item const *item) {
  return item->_properties != nullptr;
}

inline void *item_get_properties(Item const *item) {
  return item->_properties;
}

inline const char *item_get_name(Item const *item) {
  return item->_name;
}

inline uint32_t item_get_weight(Item const *item) {
  return item->_weight;
}

inline uint32_t item_get_value(Item const *item) {
  return item->_value;
}

inline ItemType item_get_type(Item const *item) {
  return item->_type;
}

inline bool item_has_coords(Item const *item) {
  return item->_coords != nullptr;
}

inline Point const *item_get_coords(Item const *item) {
  return item->_coords;
}

inline uint8_t weapon_get_hands(WeaponProperties const *weapon) {
  return weapon->_hands;
}

inline uint32_t weapon_get_attack_power(WeaponProperties const *weapon) {
  return weapon->_attack_power;
}

inline uint8_t weapon_get_life_points(WeaponProperties const *weapon) {
  return weapon->_life_points;
}

inline uint8_t tool_get_hands(ToolProperties const *tool) {
  return tool->_hands;
}

inline uint8_t tool_get_life_points(ToolProperties const *tool) {
  return tool->_life_points;
}

inline uint32_t armor_get_defense_value(ArmorProperties const *armor) {
  return armor->_defense_value;
}

inline uint8_t armor_get_life_points(ArmorProperties const *armor) {
  return armor->_life_points;
}

inline uint8_t armor_get_armor_class(ArmorProperties const *armor) {
  return armor->_armor_class;
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

void item_deserialize_check_map(msgpack_object_map const *msgpack_map) {
  LOG_INFO("Item map validation", 0);

  // Check that the received object is indeed an item, it must contain
  // exactly 6 fields and have exactly the fields we want
  assert(msgpack_map->size == 6);
  for (uint32_t i = 0; i < 6; i++) {
    assert(msgpack_map->ptr[i].key.type == MSGPACK_OBJECT_STR);
  }

  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "type");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_STR, "name");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "weight");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_POSITIVE_INTEGER, "value");
  serde_map_assert(msgpack_map, MSGPACK_OBJECT_ARRAY, "coords");

  bool has_properties = true;
  if (serde_map_find(msgpack_map, MSGPACK_OBJECT_NIL, "properties") == nullptr) {
    if (serde_map_find(msgpack_map, MSGPACK_OBJECT_MAP, "properties") == nullptr) {
      has_properties = false;
    }
  }
  assert(has_properties);
}

void armor_deserialize_check_map(msgpack_object_map const *msgmap) {
  LOG_INFO("Armor map validation", 0);

  assert(msgmap != nullptr);
  assert(msgmap->size == 3);
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "defense_value");
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "armor_class");
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "life_points");
}

ArmorProperties *armor_deserialize(msgpack_object_map const *msgmap) {
  LOG_INFO("Deserializing armor properties", 0);
  armor_deserialize_check_map(msgmap);

  ArmorProperties *props = calloc(1, sizeof(ArmorProperties));
  props->_armor_class = *(uint8_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "armor_class");
  props->_life_points = *(uint8_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "life_points");
  props->_defense_value = *(uint32_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "defense_value");

  return props;
}

void tool_deserialize_check_map(msgpack_object_map const *msgmap) {
  LOG_INFO("Tool map validation", 0);

  assert(msgmap != nullptr);
  assert(msgmap->size == 2);
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "hands");
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "life_points");
}

ToolProperties *tool_deserialize(msgpack_object_map const *msgmap) {
  LOG_INFO("Deserializing tool properties", 0);
  tool_deserialize_check_map(msgmap);

  ToolProperties *props = calloc(1, sizeof(ToolProperties));
  props->_hands = *(uint32_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "hands");
  props->_life_points = *(uint8_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "life_points");

  return props;
}

void weapon_deserialize_check_map(msgpack_object_map const *msgmap) {
  LOG_INFO("Weapon map validation", 0);

  assert(msgmap != nullptr);
  assert(msgmap->size == 3);
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "attack_power");
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "life_points");
  serde_map_assert(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "hands");
}

WeaponProperties *weapon_deserialize(msgpack_object_map const *msgmap) {
  LOG_INFO("Deserializing weapon properties", 0);
  weapon_deserialize_check_map(msgmap);

  WeaponProperties *props = calloc(1, sizeof(ArmorProperties));
  props->_attack_power = *(uint8_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "attack_power");
  props->_life_points = *(uint8_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "life_points");
  props->_hands = *(uint8_t *)serde_map_get(msgmap, MSGPACK_OBJECT_POSITIVE_INTEGER, "hands");

  return props;
}

void armor_serialize(Item const *item, msgpack_packer *packer) {
  LOG_INFO("Packing armor", 0);
  ArmorProperties *properties = (ArmorProperties *)item->_properties;
  msgpack_pack_map(packer, 3);

  serde_pack_str(packer, "defense_value");
  msgpack_pack_uint32(packer, properties->_defense_value);

  serde_pack_str(packer, "armor_class");
  msgpack_pack_uint8(packer, properties->_armor_class);

  serde_pack_str(packer, "life_points");
  msgpack_pack_uint8(packer, properties->_life_points);
}

void tool_serialize(Item const *item, msgpack_packer *packer) {
  LOG_INFO("Packing tool", 0);
  ToolProperties *properties = (ToolProperties *)item->_properties;
  msgpack_pack_map(packer, 2);

  serde_pack_str(packer, "hands");
  msgpack_pack_uint8(packer, properties->_hands);

  serde_pack_str(packer, "life_points");
  msgpack_pack_uint8(packer, properties->_life_points);
}

void weapon_serialize(Item const *item, msgpack_packer *packer) {
  LOG_INFO("Packing weapon", 0);
  WeaponProperties *properties = (WeaponProperties *)item->_properties;
  msgpack_pack_map(packer, 3);

  serde_pack_str(packer, "attack_power");
  msgpack_pack_uint32(packer, properties->_attack_power);

  serde_pack_str(packer, "life_points");
  msgpack_pack_uint8(packer, properties->_life_points);

  serde_pack_str(packer, "hands");
  msgpack_pack_uint8(packer, properties->_hands);
}

void forage_serialize(Item const *item, msgpack_packer *packer) {
  LOG_WARNING("Forage items not implemented yet, cannot pack it", 0);
  msgpack_pack_nil(packer);
}

void gem_serialize(Item const *item, msgpack_packer *packer) {
  LOG_WARNING("Gem items not implemented yet, cannot pack it", 0);
  msgpack_pack_nil(packer);
}

bool weapon_is_equal(WeaponProperties const *self, WeaponProperties const *other) {
  return self->_hands == other->_hands && self->_attack_power == other->_attack_power && self->_life_points == other->_life_points;
}

bool tool_is_equal(ToolProperties const *self, ToolProperties const *other) {
  return self->_hands == other->_hands && self->_life_points == other->_life_points;
}

bool armor_is_equal(ArmorProperties const *self, ArmorProperties const *other) {
  return self->_defense_value == other->_defense_value && self->_life_points == other->_life_points &&
         self->_armor_class == other->_armor_class;
}

bool item_is_equal(Item const *self, Item const *other) {
  assert(self != nullptr && other != nullptr);
  bool name_equal = strings_equal(item_get_name(self), item_get_name(other));
  bool type_equal = item_get_type(self) == item_get_type(other);
  bool base_equal = self->_value == other->_value && self->_weight == other->_weight;
  bool props_equal = false;

  if (type_equal && name_equal && base_equal) {
    switch (item_get_type(self)) {
      case ARMOR:
        props_equal = armor_is_equal(item_get_properties(self), item_get_properties(other));
        break;
      case WEAPON:
        props_equal = weapon_is_equal(item_get_properties(self), item_get_properties(other));
        break;
      case TOOL:
        props_equal = tool_is_equal(item_get_properties(self), item_get_properties(other));
        break;
      default:
        props_equal = true;
        break;
    }
  }

  return name_equal && type_equal && base_equal && props_equal;
}

