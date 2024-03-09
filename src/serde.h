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

#ifndef __SERDE__H__
#define __SERDE__H__

#include <msgpack/object.h>
#include <msgpack/pack.h>

/*
 * Packs a string into a packer object.
 */
void serde_pack_str(msgpack_packer *, char const *);

/*
 * Returns true if the two strings are equal
 */
bool serde_str_equal(msgpack_object_str const *, char const *);

/*
 * Asserts that a `msgpack_object_str` has the right value. This is
 * just calling `assert(serde_str_equal())`
 */
void serde_assert_str(msgpack_object_str const *, char const *);

/*
 * Finds a key in a map, returns nullptr if they key does not exist or if the
 * key exists but it's not of the expected type
 */
msgpack_object_kv const *serde_map_find(msgpack_object_map const *, msgpack_object_type, char const *);

/*
 * Returns a direct pointer to the val of a kv in a msgpack map.
 * Returns nullptr if it does not exist
 */
void const *serde_map_get(msgpack_object_map const *, msgpack_object_type, char const *);

/*
 * Asserts that a given key exists in a map and it has the right type.
 * This is just calling `assert(serde_map_find())`
 */
void serde_map_assert(msgpack_object_map const *, msgpack_object_type, char const *);

#endif /* ifndef __SERDE__H__ */
