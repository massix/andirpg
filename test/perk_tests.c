#include "perk.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <msgpack/object.h>
#include <msgpack/sbuffer.h>
#include <msgpack/unpack.h>
#include <string.h>

void perk_test_constructors(void) {
  Perk *perk = perk_new(PT_ENTITY_STATS, "LifeAugment");
  CU_ASSERT_TRUE(strings_equal(perk_get_name(perk), "LifeAugment"));
  CU_ASSERT_EQUAL(perk_get_perk_type(perk), PT_ENTITY_STATS);

  msgpack_sbuffer buffer;
  msgpack_sbuffer_init(&buffer);

  perk_serialize(perk, &buffer);
  CU_ASSERT_PTR_NOT_NULL(buffer.data);
  CU_ASSERT_TRUE(buffer.size > 0);

  msgpack_unpacker unpacker;
  msgpack_unpacker_init(&unpacker, 0);
  msgpack_unpacker_reserve_buffer(&unpacker, buffer.size);
  memcpy(msgpack_unpacker_buffer(&unpacker), buffer.data, buffer.size);
  msgpack_unpacker_buffer_consumed(&unpacker, buffer.size);

  msgpack_unpacked result;
  msgpack_unpacked_init(&result);

  CU_ASSERT_EQUAL(msgpack_unpacker_next(&unpacker, &result), MSGPACK_UNPACK_SUCCESS);
  CU_ASSERT_EQUAL(result.data.type, MSGPACK_OBJECT_MAP);

  Perk *deserialized = perk_deserialize(&result.data.via.map);
  CU_ASSERT_TRUE(strings_equal(perk_get_name(deserialized), perk_get_name(perk)));
  CU_ASSERT_EQUAL(perk_get_perk_type(deserialized), perk_get_perk_type(perk));

  msgpack_sbuffer_destroy(&buffer);
  msgpack_unpacked_destroy(&result);

  perk_free(perk);
  perk_free(deserialized);
}

void perk_test_suite() {
  CU_pSuite suite = CU_add_suite("Perk Tests", nullptr, nullptr);
  CU_add_test(suite, "Perk constructors", &perk_test_constructors);
}
