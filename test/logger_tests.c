#include "logger.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <stdint.h>
#include <stdio.h>

void logger_test(void) {
  const char *file = "./logs.txt";
  Logger     *logger = logger_new(file, WARNING);
  CU_ASSERT_PTR_NOT_NULL(logger);
  CU_ASSERT_EQUAL(logger, logger_instance());

  logger_msg(logger_instance(), DEBUG, __FILE__, __LINE__, "%s is a test, answer is: %d", "This", 42);
  logger_msg(logger_instance(), INFO, __FILE__, __LINE__, "This is %s test, logger is a pointer to 0x%p", "another", logger_instance());

  LOG_WARNING("This is a %s from a %s", "test", "macro");
  LOG_INFO("Another %s from a %s", "test", "macro");
  LOG_ERROR("This is an error!", 0);
  LOG_CRITICAL("This is a critical error caused by '%s'", "an error");

  logger_free(logger);

  FILE *output_file = fopen(file, "r");

  // I should only have three lines in the final file (I don't care about getting the actual content of the file)
  uint8_t      newlines_found = 0;
  unsigned int read_character;
  while ((read_character = fgetc(output_file)) != EOF) {
    if ((char)read_character == '\n') {
      newlines_found++;
    }
  }

  CU_ASSERT_EQUAL(newlines_found, 3);

  fclose(output_file);
  unlink(file);
}

void logger_test_suite(void) {
  CU_pSuite suite = CU_add_suite("Logger Tests", nullptr, nullptr);
  CU_add_test(suite, "Basic logging", &logger_test);
}

