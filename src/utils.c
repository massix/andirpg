#include "utils.h"
#include <stdint.h>
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

