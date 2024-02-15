#include "ui_point.h"
#include <stdlib.h>

struct UiPoint {
  int _x;
  int _y;
};

UiPoint *ui_point_new(int x, int y) {
  UiPoint *ret = calloc(1, sizeof(UiPoint));
  ret->_x = x;
  ret->_y = y;

  return ret;
}

inline int ui_point_get_x(UiPoint *point) {
  return point->_x;
}

inline int ui_point_get_y(UiPoint *point) {
  return point->_y;
}

inline void ui_point_free(UiPoint *point) {
  free(point);
}

