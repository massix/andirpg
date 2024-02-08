#ifndef __CONFIGURATION__H__
#define __CONFIGURATION__H__

#include "logger.h"
#include <stdint.h>
typedef struct Configuration Configuration;

Configuration *configuration_new(const char *);
const char    *configuration_get_path(Configuration *);

// Player section
const char *configuration_get_player_name(Configuration *);
uint32_t    configuration_get_player_starting_hp(Configuration *);

// Log section
const char *configuration_get_log_output_file(Configuration *);
LogLevel    configuration_get_log_level(Configuration *);

void configuration_init(Configuration *);
void configuration_free(Configuration *);

#endif
