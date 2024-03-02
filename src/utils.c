#include "utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
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

