#ifndef __ITEM__H__
#define __ITEM__H__

#include "point.h"
#include <stdint.h>

typedef struct Item Item;

typedef struct WeaponProperties WeaponProperties;
typedef struct ToolProperties   ToolProperties;
typedef struct ArmorProperties  ArmorProperties;
typedef struct GemProperties    GemProperties;

typedef struct ForageProperties ForageProperties;
typedef struct BerryProperties  BerryProperties;
typedef struct WoodProperties   WoodProperties;

typedef enum ItemType { TOOL, WEAPON, ARMOR, FORAGE, GEM } ItemType;
typedef enum ForageType { BERRY, WOOD } ForageType;

// Build a generic Item
Item *item_new(ItemType, const char *, uint32_t weight, uint32_t value);
Item *item_clone(Item *);

bool        item_has_properties(Item *);
void       *item_get_properties(Item *);
const char *item_get_name(Item *);
uint32_t    item_get_weight(Item *);
uint32_t    item_get_value(Item *);
ItemType    item_get_type(Item *);

// By default, an item belongs to an entity, but an Item can also be present
// on the map, hence we might have coordinates (although they are not mandatory)
bool   item_has_coords(Item *);
Point *item_get_coords(Item *);
void   item_set_coords(Item *, uint32_t x, uint32_t y);

// If an item stops belonging to the map and re-belongs to an entity
void item_clear_coords(Item *);

// Builders for specific items
Item *weapon_new(const char *, uint32_t weight, uint32_t value, uint8_t hands, uint32_t attack_power, uint8_t life_points);
Item *tool_new(const char *, uint32_t weight, uint32_t value, uint8_t hands, uint8_t life_points);
Item *armor_new(const char *, uint32_t weight, uint32_t value, uint32_t defense_value, uint8_t life_points, uint8_t armor_class);

// Getters for specific properties

// WEAPON
uint8_t  weapon_get_hands(WeaponProperties *);
uint32_t weapon_get_attack_power(WeaponProperties *);
uint8_t  weapon_get_life_points(WeaponProperties *);

// TOOL
uint8_t tool_get_hands(ToolProperties *);
uint8_t tool_get_life_points(ToolProperties *);

// ARMOR
uint32_t armor_get_defense_value(ArmorProperties *);
uint8_t  armor_get_life_points(ArmorProperties *);
uint8_t  armor_get_armor_class(ArmorProperties *);

void item_free(Item *);

#endif

