#ifndef __UI__MAP_WINDOW__H__
#define __UI__MAP_WINDOW__H__

#include "map.h"
#include "ui_point.h"
#include <ncurses.h>

typedef struct MapWindow MapWindow;

MapWindow *map_window_new(Map *, int lines, int cols, int x, int y);
WINDOW    *map_window_get_ncurses_window(MapWindow *);
void       map_window_draw(MapWindow *);
UiPoint   *map_window_get_ui_point(MapWindow *);
int        map_window_get_lines(MapWindow *);
int        map_window_get_cols(MapWindow *);

void map_window_free(MapWindow *);

#endif // !__UI__MAP_WINDOW__H__
