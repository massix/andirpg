// AndiRPG -- Name not final
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

#ifndef __PERKS__H__
#define __PERKS__H__

#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
typedef struct Perk Perk;

typedef enum PerkType {
  PT_ENTITY_STATS, /* Perks interacting with Entities stats */
  PT_ENVIRONMENT,  /* Perks interacting with the environment */
  PT_ITEMS_STATS,  /* Perks interacting with Items stats */
} PerkType;

// Constructors and deconstructors
Perk *perk_new(PerkType, const char *);
void  perk_serialize(Perk const *, msgpack_sbuffer *);
Perk *perk_deserialize(msgpack_object_map const *);
void  perk_free(Perk *);

// Getters and setters
char const *perk_get_name(Perk const *);
PerkType    perk_get_perk_type(Perk const *);

#endif /* ifndef __PERKS__H__ */
