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

#include "utils.h"
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline bool strings_equal(const char *lhs, const char *rhs) {
  size_t lhs_size = strlen(lhs);
  size_t rhs_size = strlen(rhs);
  return (lhs_size == rhs_size) && (strncmp(lhs, rhs, lhs_size) == 0);
}

inline uint32_t min(uint32_t lhs, uint32_t rhs) {
  return lhs < rhs ? lhs : rhs;
}

inline uint32_t max(uint32_t lhs, uint32_t rhs) {
  return lhs > rhs ? lhs : rhs;
}

inline int64_t file_size(FILE *file) {
  assert(file != nullptr);
  int64_t length = -1;

  // Store the current position
  int64_t current_pos = ftell(file);

  // Go to the end of file
  fseek(file, 0, SEEK_END);
  length = ftell(file);

  // Restore the old position
  fseek(file, current_pos, SEEK_SET);

  return length;
}

void panic(const char *fmt, ErrorCode error_code, ...) {
  va_list list;
  va_start(list, error_code);
  fprintf(stderr, "\n PANIC (%d): \"", error_code);
  vfprintf(stderr, fmt, list);
  fprintf(stderr, "\"\n");
  va_end(list);

  exit(error_code);
}

