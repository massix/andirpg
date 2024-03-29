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

#include "configuration.h"
#include "logger.h"
#include "utils.h"
#include <ini.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Configuration {
  char *_file_path;

  // Player section
  char    *_player_name;
  uint32_t _player_starting_hp;

  // Log section
  char    *_log_output_file;
  LogLevel _log_level;
};

Configuration *configuration_new(const char *path) {
  Configuration *ret = calloc(1, sizeof(Configuration));
  ret->_file_path = strdup(path);

  // Provide some default values
  ret->_player_starting_hp = 10;
  ret->_player_name = strdup("NoName");
  ret->_log_output_file = strdup("./game.log");
  ret->_log_level = INFO;
  return ret;
}

inline const char *configuration_get_path(Configuration const *configuration) {
  return configuration->_file_path;
}

inline const char *configuration_get_player_name(Configuration const *configuration) {
  return configuration->_player_name;
}

inline uint32_t configuration_get_player_starting_hp(Configuration const *configuration) {
  return configuration->_player_starting_hp;
}

inline const char *configuration_get_log_output_file(Configuration const *configuration) {
  return configuration->_log_output_file;
}

inline LogLevel configuration_get_log_level(Configuration const *configuration) {
  return configuration->_log_level;
}

static int configuration_handler(void *input, const char *section, const char *name, const char *value) {
  Configuration *conf = (Configuration *)input;

  if (strings_equal(section, "player")) {
    if (strings_equal(name, "name")) {
      conf->_player_name = strdup(value);
    }

    if (strings_equal(name, "starting_hp")) {
      conf->_player_starting_hp = atoi(value);
    }
  }

  if (strings_equal(section, "log")) {
    if (strings_equal(name, "output_file")) {
      conf->_log_output_file = strdup(value);
    }

    if (strings_equal(name, "level")) {
      if (strings_equal(value, "DEBUG")) {
        conf->_log_level = DEBUG;
      } else if (strings_equal(value, "NOTICE")) {
        conf->_log_level = NOTICE;
      } else if (strings_equal(value, "WARNING")) {
        conf->_log_level = WARNING;
      } else if (strings_equal(value, "ERROR")) {
        conf->_log_level = ERROR;
      } else if (strings_equal(value, "CRITICAL")) {
        conf->_log_level = CRITICAL;
      } else {
        conf->_log_level = INFO;
      }
    }
  }

  return 1;
}

void configuration_init(Configuration *configuration) {
  FILE *input_file = fopen(configuration->_file_path, "rb");
  if (input_file != nullptr) {
    fclose(input_file);
    ini_parse(configuration->_file_path, &configuration_handler, configuration);
  }
}

void configuration_free(Configuration *configuration) {
  free(configuration->_file_path);
  free(configuration->_player_name);
  free(configuration->_log_output_file);
  free(configuration);
}
