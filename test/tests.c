#include "logger.h"
#include <CUnit/Basic.h>
#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <stdio.h>
#include <sys/types.h>

// forward decl
void entity_test_suite();
void map_test_suite();
void engine_test_suite();
void configuration_test_suite();
void item_test_suite();
void logger_test_suite();

int main(int argc, char *argv[]) {
  logger_new("./tests.log", DEBUG);

  LOG_INFO("Beginning tests", 0);

  CU_initialize_registry();
  int number_of_failures = -1;

  entity_test_suite();
  map_test_suite();
  engine_test_suite();
  configuration_test_suite();
  item_test_suite();
  logger_test_suite();

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_ErrorCode code = CU_basic_run_tests();

  if (code != CUE_SUCCESS) {
    fprintf(stderr, "Failure while trying to launch the tests: Error Code: %d\n", code);
  } else {
    number_of_failures = (int)CU_get_number_of_failures();
  }

  CU_cleanup_registry();

  logger_free(logger_instance());

  return number_of_failures;
}
