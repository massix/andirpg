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

#ifndef __ENGINE__H__
#define __ENGINE__H__

#include "entity.h"
#include "map.h"
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Engine Engine;

Engine *engine_new(Map *);
Engine *engine_deserialize(msgpack_object_map const *);

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
void     engine_serialize(Engine *, msgpack_sbuffer *);

// Fetch all the entities close to the active one
Entity **engine_get_close_entities(Engine *, ssize_t *);
void     engine_free(Engine *);

#endif
