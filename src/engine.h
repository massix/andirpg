#ifndef __ENGINE__H__
#define __ENGINE__H__

#include "entity.h"
#include "map.h"
#include <stdint.h>
#include <sys/types.h>

typedef struct Engine Engine;

Engine *engine_new(Map *);

Map     *engine_get_map(Engine *);
void     engine_add_entity(Engine *, Entity *);
void     engine_entity_attack(Engine *, Entity *, Entity *);
void     engine_set_active_entity(Engine *, const char *);
Entity  *engine_get_active_entity(Engine *);
void     engine_clear_active_entity(Engine *);
void     engine_handle_keypress(Engine *, char);
bool     engine_has_active_entity(Engine *);
void     engine_move_all_entities(Engine *);
uint32_t engine_get_current_cycle(Engine *);

// Fetch all the entities close to the active one
Entity **engine_get_close_entities(Engine *, ssize_t *);
void     engine_free(Engine *);

#endif
