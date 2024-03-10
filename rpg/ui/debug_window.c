// andirpg -- description to be modified
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

#include "debug_window.h"
#include "boxed_window.h"
#include "engine.h"
#include "entity.h"
#include "map.h"
#include "point.h"
#include "tile.h"
#include <ncurses.h>
#include <stdlib.h>
#include <sys/types.h>

struct DebugWindow {
  BoxedWindow *_window;
  Engine      *_engine; // the engine is owned by the main application
  bool         _visible;
};

DebugWindow *debug_window_new(Engine *engine, int lines, int cols, int y, int x) {
  DebugWindow *ret = calloc(1, sizeof(DebugWindow));
  ret->_engine = engine;
  ret->_visible = false;

  struct BoxedWindowOptions options;
  boxed_window_options_default(&options);

  ret->_window = boxed_window_new(&options, "Debug", lines, cols, y, x, nullptr);

  return ret;
}

bool debug_window_is_visible(DebugWindow *dbg) {
  return dbg->_visible;
}

void debug_window_show(DebugWindow *dbg) {
  dbg->_visible = true;
}

void debug_window_hide(DebugWindow *dbg) {
  dbg->_visible = false;
  boxed_window_clear(dbg->_window);
}

void debug_window_draw(DebugWindow *dbg) {
  if (debug_window_is_visible(dbg)) {
    WINDOW *target = boxed_window_get_inner_window(dbg->_window);
    boxed_window_draw(dbg->_window);
    ssize_t  s_movable_entities;
    Entity **movable_entities = map_filter_entities(engine_get_map(dbg->_engine), &entity_can_move, &s_movable_entities);
    free(movable_entities);

    Point const *current_coords = entity_get_coords(engine_get_active_entity(dbg->_engine));
    Tile const  *current_tile = map_get_tile(engine_get_map(dbg->_engine), point_get_x(current_coords), point_get_y(current_coords));

    wclear(target);
    wmove(target, 0, 0);
    wprintw(target, "Current cycle: %d\n", engine_get_current_cycle(dbg->_engine));
    wprintw(target, "Entities: %d\n", map_count_entities(engine_get_map(dbg->_engine)));
    wprintw(target, "Can move: %zd\n", s_movable_entities);
    wprintw(target, "Items: %d\n", map_count_items(engine_get_map(dbg->_engine)));
    wprintw(target, "Coords: %d, %d\n", point_get_x(current_coords), point_get_y(current_coords));
    wprintw(target, "Tile - Kind  : %c\n", tile_get_tile_kind(current_tile));
    wprintw(target, "Tile - Light : %d\n", tile_get_base_light(current_tile));
    wprintw(target, "Tile - Noise : %d\n", tile_get_base_noise(current_tile));
    wprintw(target, "Tile - Inside: %s\n", tile_is_inside(current_tile) ? "yes" : "no");
    wprintw(target, "Tile - Traver: %s\n", tile_is_traversable(current_tile) ? "yes" : "no");
    wnoutrefresh(target);
  }
}

void debug_window_free(DebugWindow *dbg) {
  boxed_window_free(dbg->_window);
  free(dbg);
}

