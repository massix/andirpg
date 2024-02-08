#ifndef __ENTITY__H__
#define __ENTITY__H__

#include "item.h"
#include <point.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Entity Entity;

typedef enum EntityType { HUMAN = 0, ANIMAL = 1, INHUMAN = 2, TREE = 3, MOUNTAIN = 4 } EntityType;

const char *entity_type_to_string(EntityType);
char        entity_type_to_char(EntityType);

Entity *entity_new(uint32_t starting_lp, EntityType type, const char *name, uint32_t start_x, uint32_t start_y);

Entity  *entity_from_string(const char *);
char    *entity_to_string(Entity *);
bool     entity_can_move(Entity *);
void     entity_move(Entity *, uint32_t delta_x, uint32_t delta_y);
void     entity_hurt(Entity *, uint32_t life_points);
void     entity_heal(Entity *, uint32_t life_points);
void     entity_resurrect(Entity *);
uint32_t entity_get_life_points(Entity *);
uint32_t entity_get_starting_life_points(Entity *);

Point      *entity_get_coords(Entity *);
bool        entity_is_alive(Entity *);
bool        entity_is_dead(Entity *);
EntityType *entity_get_entity_type(Entity *);
const char *entity_get_name(Entity *);

// Inventory manipulation
size_t entity_inventory_count(Entity *);
void   entity_inventory_add_item(Entity *, Item *);
void   entity_inventory_remove_item(Entity *, const char *);
void   entity_inventory_clear(Entity *);
Item **entity_inventory_filter(Entity *, bool (*)(Item *), ssize_t *);
Item **entity_inventory_get(Entity *); // PERF: Only useful for tests

void entity_free(Entity *);

#endif
