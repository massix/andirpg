#ifndef __MAP__H__
#define __MAP__H__

#include "entity.h"
#include "item.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

typedef struct Map Map;
typedef struct MapBoundaries {
  uint32_t x;
  uint32_t y;
} MapBoundaries;

Map *map_new(uint32_t x_size, uint32_t y_size, uint32_t max_entities);
void map_dump_to_file(Map *, const char *path);
Map *map_from_string(const char *);

void     map_add_entity(Map *, Entity *);
void     map_add_item(Map *, Item *, uint32_t x, uint32_t y);
void     map_remove_item(Map *, const char *);
bool     map_contains_item(Map *, const char *);
uint32_t map_count_items(Map *);
Item    *map_get_item(Map *, const char *);
Entity  *map_get_entity(Map *, const char *);
Entity **map_get_all_entities(Map *);
Entity **map_filter_entities(Map *, bool (*)(Entity *), ssize_t *);
void     map_remove_entity(Map *, const char *);
int      map_get_index_of_entity(Map *, const char *);
bool     map_contains_entity(Map *, const char *);
int      map_count_entities(Map *);
void     map_fdraw(Map *, FILE *);
void     map_draw(Map *);
void     map_wdraw(Map *, WINDOW *);
bool     map_is_tile_free(Map *, uint32_t x, uint32_t y);

MapBoundaries map_get_boundaries(Map *);

void map_free(Map *);

#endif
