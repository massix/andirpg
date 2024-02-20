#include "player_window.h"
#include "boxed_window.h"
#include "entity.h"
#include "logger.h"
#include "ui_point.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

struct PlayerWindow {
  Entity      *_entity;
  BoxedWindow *_window;
  UiPoint     *_point;
};

PlayerWindow *player_window_new(Entity *entity, int lines, int cols, int x, int y) {
  if (lines < 4 || cols < (strlen(entity_get_name(entity)))) {
    LOG_CRITICAL("Winsize (%dx%d) is too small to draw player info", cols, lines);
    return nullptr;
  }

  LOG_DEBUG("Player Window: lines: %d, cols: %d, x: %d, y: %d", lines, cols, x, y);

  PlayerWindow *ret = calloc(1, sizeof(PlayerWindow));

  ret->_entity = entity;
  ret->_point = ui_point_new(x, y);

  struct BoxedWindowOptions options;
  boxed_window_options_default(&options);

  ret->_window = boxed_window_new(&options, entity_get_name(entity), lines, cols, y, x, nullptr);

  return ret;
}

void player_window_draw_inner(PlayerWindow *plw) {
  WINDOW *target = boxed_window_get_inner_window(plw->_window);
  wclear(target);
  mvwprintw(target, 0, 0, "HP: %d/%d", entity_get_life_points(plw->_entity), entity_get_starting_life_points(plw->_entity));
  mvwprintw(target, 1, 0, "XP: 0/1000");
  mvwprintw(target, 2, 0, "LV: 0");
  mvwprintw(target, 3, 0, "Perks: No active perks");
  wnoutrefresh(target);
}

void player_window_draw(PlayerWindow *plw) {
  boxed_window_draw(plw->_window);
  player_window_draw_inner(plw);
}

inline int player_window_get_cols(PlayerWindow *plw) {
  return boxed_window_get_cols(plw->_window);
}

inline int player_window_get_lines(PlayerWindow *plw) {
  return boxed_window_get_lines(plw->_window);
}

void player_window_free(PlayerWindow *player_window) {
  boxed_window_free(player_window->_window);
  free(player_window);
}

