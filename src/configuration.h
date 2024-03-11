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

#ifndef __CONFIGURATION__H__
#define __CONFIGURATION__H__

#include "logger.h"
#include <stdint.h>
typedef struct Configuration Configuration;

Configuration *configuration_new(const char *);
const char    *configuration_get_path(Configuration const *);

// Player section
const char *configuration_get_player_name(Configuration const *);
uint32_t    configuration_get_player_starting_hp(Configuration const *);

// Log section
const char *configuration_get_log_output_file(Configuration const *);
LogLevel    configuration_get_log_level(Configuration const *);

void configuration_init(Configuration *);
void configuration_free(Configuration *);

#endif
