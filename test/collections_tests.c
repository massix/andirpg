#include "collections/linked_list.h"
#include "entity.h"
#include "item.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void linked_list_zero_items(void) {
  LinkedList *list = linked_list_new(0, (FreeFunction)&item_free);
  CU_ASSERT_TRUE(linked_list_is_empty(list));
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 0);

  linked_list_add(list, armor_new("An armor", 10, 10, 10, 10, 10));
  CU_ASSERT_EQUAL(linked_list_count(list), 1);
  CU_ASSERT_FALSE(linked_list_is_empty(list));
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 1);

  linked_list_add(list, armor_new("Another armor", 10, 10, 10, 10, 10));
  CU_ASSERT_EQUAL(linked_list_count(list), 2);
  CU_ASSERT_FALSE(linked_list_is_empty(list));
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 2);

  linked_list_remove(list, 1);
  CU_ASSERT_EQUAL(linked_list_count(list), 1);
  CU_ASSERT_FALSE(linked_list_is_empty(list));

  linked_list_remove(list, 0);
  CU_ASSERT_EQUAL(linked_list_count(list), 0);
  CU_ASSERT_TRUE(linked_list_is_empty(list));

  // The list does not shrink automatically
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 2);

  linked_list_free(list);
}

// Used to retrieve a specific entity
bool get_entity_444(Entity const *entity) {
  return strings_equal(entity_get_name(entity), "Entity #444");
}

// Used to retrieve all the entities from 200 to 299
bool get_entity_2xx(Entity const *entity) {
  const char *cmp = "Entity #2";
  return strncmp(entity_get_name(entity), cmp, strlen(cmp)) == 0;
}

void linked_list_lot_items(void) {
  LinkedList    *list = linked_list_new(512, (FreeFunction)&entity_free);
  EntityBuilder *builder = entity_builder_new();
  builder->with_type(builder, INHUMAN);

  CU_ASSERT_TRUE(linked_list_is_empty(list));

  for (uint i = 0; i < 512; i++) {
    char *entity_name = malloc(50 * sizeof(char));
    sprintf(entity_name, "Entity #%03d", i);
    linked_list_add(list, builder->with_xp(builder, i)->with_name(builder, entity_name)->build(builder, false));

    free(entity_name);
  }

  CU_ASSERT_FALSE(linked_list_is_empty(list));
  CU_ASSERT_EQUAL(linked_list_count(list), 512);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 512);

  Entity *entity = linked_list_get(list, 30);
  CU_ASSERT_PTR_NOT_NULL(entity);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(entity), "Entity #030"));

  // Removing the first entity should get "Entity #1" in first place
  linked_list_remove(list, 0);

  Entity *new_first = linked_list_get(list, 0);
  CU_ASSERT_PTR_NOT_NULL(new_first);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(new_first), "Entity #001"));

  // Now if we add an entity, it should be the last one and the size should stay the same
  linked_list_add(list, builder->with_name(builder, "Hello from last position")->build(builder, false));

  Entity *last = linked_list_get(list, 511);
  CU_ASSERT_PTR_NOT_NULL(last);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(last), "Hello from last position"));
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 512);

  // Removing the last entity should not move other entities
  linked_list_remove(list, 511);
  new_first = linked_list_get(list, 0);
  CU_ASSERT_PTR_NOT_NULL(new_first);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(new_first), "Entity #001"));

  Entity *middle = linked_list_get(list, 216);
  CU_ASSERT_PTR_NOT_NULL(middle);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(middle), "Entity #217"));

  // Removing an entity in the middle should shift all the other ones but not the preceding
  linked_list_remove(list, 216);

  // From 0 to 215 we are still shifted by 1 (because of removing the first)
  new_first = linked_list_get(list, 0);
  CU_ASSERT_PTR_NOT_NULL(new_first);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(new_first), "Entity #001"));

  entity = linked_list_get(list, 200);
  CU_ASSERT_PTR_NOT_NULL(entity);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(entity), "Entity #201"));

  // From 216 onwards, the shift is by 2
  middle = linked_list_get(list, 216);
  CU_ASSERT_PTR_NOT_NULL(middle);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(middle), "Entity #218"));

  Entity *near_to_last = linked_list_get(list, 500);
  CU_ASSERT_PTR_NOT_NULL(near_to_last);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(near_to_last), "Entity #502"));

  entity_builder_free(builder);

  // We can also retrieve the entity #444
  Entity *e444 = linked_list_find(list, (Comparator)&get_entity_444);
  CU_ASSERT_PTR_NOT_NULL(e444);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(e444), "Entity #444"));

  // We can also retrieve a list of entities
  size_t   list_size = 0;
  Entity **e2xx = (Entity **)linked_list_find_all(list, (Comparator)&get_entity_2xx, &list_size);
  CU_ASSERT_PTR_NOT_NULL(e2xx);
  CU_ASSERT_PTR_NOT_NULL(e2xx[0]);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(e2xx[0]), "Entity #200"));

  // 99 elements because we've removed element #217 before
  CU_ASSERT_EQUAL(list_size, 99);
  CU_ASSERT_TRUE(strings_equal(entity_get_name(e2xx[98]), "Entity #299"));
  CU_ASSERT_PTR_NULL(e2xx[99]);

  // Drawback, we must free the list ourselves
  free(e2xx);

  // We can also iterate over the list
  uint iterations = 0;
  last = linked_list_get(list, linked_list_count(list) - 1);
  Entity const *last_iterated = nullptr;
  for (Entity *current = linked_list_iterator_next(list); current != nullptr; current = linked_list_iterator_next(list)) {
    iterations++;
    last_iterated = current;
  }

  linked_list_iterator_reset(list);
  CU_ASSERT_EQUAL(iterations, linked_list_count(list));
  CU_ASSERT_EQUAL(last_iterated, last);

  // We can empty the list simply by always removing the first element
  while (!linked_list_is_empty(list)) {
    linked_list_remove(list, 0);
  }

  CU_ASSERT_TRUE(linked_list_is_empty(list));
  CU_ASSERT_PTR_NULL(linked_list_get(list, 0));

  linked_list_free(list);
}

void linked_list_memory(void) {
  LinkedList *list = linked_list_new(0, &free);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 0);
  CU_ASSERT_TRUE(linked_list_is_empty(list));

  linked_list_memory_extend(list, 512);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 512);
  CU_ASSERT_TRUE(linked_list_is_empty(list));

  linked_list_memory_shrink(list);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 0);
  CU_ASSERT_TRUE(linked_list_is_empty(list));

  linked_list_add(list, malloc(10 * sizeof(char)));
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 1);
  CU_ASSERT_FALSE(linked_list_is_empty(list));

  linked_list_memory_extend(list, 31);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 32);
  CU_ASSERT_FALSE(linked_list_is_empty(list));

  linked_list_memory_shrink(list);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 1);
  CU_ASSERT_FALSE(linked_list_is_empty(list));

  linked_list_remove(list, 0);
  linked_list_memory_shrink(list);
  CU_ASSERT_EQUAL(linked_list_get_current_size(list), 0);
  CU_ASSERT_TRUE(linked_list_is_empty(list));

  linked_list_free(list);
}

void collection_test_suite() {
  CU_pSuite suite = CU_add_suite("Collections Tests", nullptr, nullptr);
  CU_add_test(suite, "Linked Lists: Add and remove, list with 0 items", &linked_list_zero_items);
  CU_add_test(suite, "Linked Lists: Add and remove, lots of items", &linked_list_lot_items);
  CU_add_test(suite, "Linked Lists: Memory management", &linked_list_memory);
}
