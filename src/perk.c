#include "perk.h"
#include "serde.h"
#include <msgpack/object.h>
#include <msgpack/pack.h>
#include <msgpack/sbuffer.h>
#include <stdlib.h>
#include <string.h>

struct Perk {
  PerkType _type;
  char    *_name;
};

Perk *perk_new(PerkType type, const char *name) {
  Perk *perk = calloc(1, sizeof(Perk));
  perk->_type = type;
  perk->_name = strdup(name);
  return perk;
}

void perk_serialize(Perk const *self, msgpack_sbuffer *buffer) {
  msgpack_packer *packer = msgpack_packer_new(buffer, &msgpack_sbuffer_write);

  msgpack_pack_map(packer, 2);
  serde_pack_str(packer, "type");
  msgpack_pack_uint8(packer, self->_type);

  serde_pack_str(packer, "name");
  serde_pack_str(packer, self->_name);

  msgpack_packer_free(packer);
}

Perk *perk_deserialize(msgpack_object_map const *map) {
  serde_map_assert(map, MSGPACK_OBJECT_STR, "name");
  serde_map_assert(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "type");

  Perk                     *ret_val = calloc(1, sizeof(Perk));
  msgpack_object_str const *name = serde_map_get(map, MSGPACK_OBJECT_STR, "name");
  ret_val->_name = malloc(name->size);
  memcpy(ret_val->_name, name->ptr, name->size);

  ret_val->_type = *(PerkType const *)serde_map_get(map, MSGPACK_OBJECT_POSITIVE_INTEGER, "type");

  return ret_val;
}

void perk_free(Perk *self) {
  free(self->_name);
  free(self);
}

char const *perk_get_name(Perk const *self) {
  return self->_name;
}

PerkType perk_get_perk_type(Perk const *self) {
  return self->_type;
}
