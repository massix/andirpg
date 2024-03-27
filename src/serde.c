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

#include "serde.h"
#include <assert.h>
#include <malloc.h>
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <string.h>
#include <sys/types.h>

inline void serde_pack_str(msgpack_packer *packer, char const *str) {
  msgpack_pack_str_with_body(packer, str, strlen(str));
}

bool serde_str_equal(msgpack_object_str const *mp_str, char const *str) {
  return strlen(str) == mp_str->size && strncmp(mp_str->ptr, str, mp_str->size) == 0;
}

inline void serde_assert_str(msgpack_object_str const *mp_str, char const *str) {
  assert(serde_str_equal(mp_str, str));
}

msgpack_object_kv const *serde_map_find(msgpack_object_map const *mp_map, msgpack_object_type exp_type, char const *key) {
  msgpack_object_kv *ret = nullptr;
  for (uint i = 0; i < mp_map->size; i++) {
    if (mp_map->ptr[i].key.type == MSGPACK_OBJECT_STR && serde_str_equal(&mp_map->ptr[i].key.via.str, key) &&
        mp_map->ptr[i].val.type == exp_type) {
      ret = &mp_map->ptr[i];
      break;
    }
  }

  return ret;
}

msgpack_object_kv const *serde_map_find_l(msgpack_object_map const *mp_map, char const *key) {
  msgpack_object_kv *ret = nullptr;
  for (uint i = 0; i < mp_map->size; i++) {
    if (mp_map->ptr[i].key.type == MSGPACK_OBJECT_STR && serde_str_equal(&mp_map->ptr[i].key.via.str, key)) {
      ret = &mp_map->ptr[i];
      break;
    }
  }

  return ret;
}

void const *serde_map_get(msgpack_object_map const *map, msgpack_object_type type, char const *key) {
  void const *ret = nullptr;

  msgpack_object_kv const *obj = serde_map_find(map, type, key);
  if (obj != nullptr) {
    switch (type) {
      case MSGPACK_OBJECT_MAP:
        ret = &obj->val.via.map;
        break;
      case MSGPACK_OBJECT_POSITIVE_INTEGER:
        ret = &obj->val.via.u64;
        break;
      case MSGPACK_OBJECT_NEGATIVE_INTEGER:
        ret = &obj->val.via.i64;
        break;
      case MSGPACK_OBJECT_STR:
        ret = &obj->val.via.str;
        break;
      case MSGPACK_OBJECT_BOOLEAN:
        ret = &obj->val.via.boolean;
        break;
      case MSGPACK_OBJECT_ARRAY:
        ret = &obj->val.via.array;
        break;
      case MSGPACK_OBJECT_FLOAT:
        ret = &obj->val.via.f64;
        break;
      default:
        break;
    }
  }

  return ret;
}

inline void serde_map_assert(msgpack_object_map const *mp_map, msgpack_object_type exp_type, char const *key) {
  assert(serde_map_find(mp_map, exp_type, key) != nullptr);
}

