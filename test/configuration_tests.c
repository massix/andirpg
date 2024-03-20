#include "configuration.h"
#include "logger.h"
#include "utils.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <ini.h>

void configuration_new_test(void) {
  Configuration *configuration = configuration_new("./andirpg.ini");
  CU_ASSERT_PTR_NOT_NULL(configuration);

  CU_ASSERT_TRUE(strings_equal(configuration_get_path(configuration), "./andirpg.ini"));

  configuration_init(configuration);

  // Without initing it should only have the default values
  CU_ASSERT_EQUAL(configuration_get_player_starting_hp(configuration), 10);
  CU_ASSERT_TRUE(strings_equal(configuration_get_player_name(configuration), "NoName"));

  configuration_free(configuration);
}

void configuration_parse_test(void) {
  Configuration *configuration = configuration_new("./test/test.ini");
  configuration_init(configuration);

  CU_ASSERT_PTR_NOT_NULL(configuration_get_player_name(configuration));
  CU_ASSERT_TRUE(strings_equal(configuration_get_player_name(configuration), "Avatar"));
  CU_ASSERT_EQUAL(configuration_get_player_starting_hp(configuration), 50);
  CU_ASSERT_TRUE(strings_equal(configuration_get_log_output_file(configuration), "./my_game.log"));
  CU_ASSERT_EQUAL(configuration_get_log_level(configuration), ERROR);

  configuration_free(configuration);
}

void configuration_file_does_not_exist(void) {
  Configuration *configuration = configuration_new("./test/not_exists.ini");
  configuration_init(configuration);

  // Without initing it should only have the default values
  CU_ASSERT_EQUAL(configuration_get_player_starting_hp(configuration), 10);
  CU_ASSERT_TRUE(strings_equal(configuration_get_player_name(configuration), "NoName"));

  configuration_free(configuration);
}

void configuration_test_suite() {
  CU_pSuite suite = CU_add_suite("Configuration Tests", nullptr, nullptr);
  CU_add_test(suite, "New configuration", &configuration_new_test);
  CU_add_test(suite, "Parse configuration", &configuration_parse_test);
  CU_add_test(suite, "File does not exist", &configuration_file_does_not_exist);
}
