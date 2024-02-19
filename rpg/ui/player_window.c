#include "player_window.h"
#include "entity.h"
#include "logger.h"
#include "ui_point.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PlayerWindow {
  Entity  *_entity;
  WINDOW  *_main_window;
  WINDOW  *_inner_window;
  UiPoint *_point;
  int      _cols;
  int      _lines;
  int      _max_x;
  int      _max_y;
};

PlayerWindow *player_window_new(Entity *entity, int lines, int cols, int x, int y) {
  if (lines < 4 || cols < (strlen(entity_get_name(entity)))) {
    LOG_CRITICAL("Winsize (%dx%d) is too small to draw player info", cols, lines);
    return nullptr;
  }

  LOG_DEBUG("Player Window: lines: %d, cols: %d, x: %d, y: %d", lines, cols, x, y);

  PlayerWindow *ret = calloc(1, sizeof(PlayerWindow));
  ret->_entity = entity;
  ret->_cols = cols;
  ret->_lines = lines;
  ret->_point = ui_point_new(x, y);

  ret->_main_window = newwin(lines, cols, y, x);
  ret->_inner_window = subwin(ret->_main_window, 4, 30, y + 2, x + 2);

  // Help with the coordinates
  ret->_max_x = getmaxx(ret->_main_window);
  ret->_max_y = getmaxy(ret->_main_window);
  LOG_DEBUG("Coords, X: %d-%d Y: %d-%d", ret->_main_window->_begx, ret->_max_x, ret->_main_window->_begy, ret->_max_y);

  return ret;
}

void player_window_free(PlayerWindow *player_window) {
  delwin(player_window->_inner_window);
  delwin(player_window->_main_window);
  free(player_window);
}

void player_window_draw_inner(PlayerWindow *plw) {
  wclear(plw->_inner_window);
  mvwprintw(plw->_inner_window, 0, 0, "HP: %d/%d", entity_get_life_points(plw->_entity), entity_get_starting_life_points(plw->_entity));
  mvwprintw(plw->_inner_window, 1, 0, "XP: 0/1000");
  mvwprintw(plw->_inner_window, 2, 0, "LV: 0");
  mvwprintw(plw->_inner_window, 3, 0, "Perks: No active perks");
  wrefresh(plw->_inner_window);
}

void player_window_draw(PlayerWindow *plw) {
  int cur_x = 0;
  int cur_y = 0;

  // Draw the four corners
  mvwprintw(plw->_main_window, 0, 0, "+");
  mvwprintw(plw->_main_window, plw->_max_y - 1, 0, "+");
  mvwprintw(plw->_main_window, 0, plw->_max_x - 1, "+");
  mvwprintw(plw->_main_window, plw->_max_y - 1, plw->_max_x - 1, "+");

  // Draw the top-left to top-right border
  // Draw the bottom-left bottom-right border
  for (cur_x = 1; cur_x < plw->_max_x - 1; cur_x++) {
    mvwprintw(plw->_main_window, 0, cur_x, "-");
    mvwprintw(plw->_main_window, plw->_max_y - 1, cur_x, "-");
  }

  // Draw the top-left to bottom-left border
  // Draw the top-right to bottom-right border
  for (cur_y = 1; cur_y < plw->_max_y - 1; cur_y++) {
    mvwprintw(plw->_main_window, cur_y, 0, "|");
    mvwprintw(plw->_main_window, cur_y, plw->_max_x - 1, "|");
  }

  // Draw the name of the player on the top corner, right-aligned
  char *player_name = calloc(strlen(entity_get_name(plw->_entity)) + 10, sizeof(char));
  sprintf(player_name, "[ %s ]", entity_get_name(plw->_entity));

  mvwprintw(plw->_main_window, 0, plw->_max_x - strlen(player_name) - 3, "%s", player_name);

  free(player_name);

  wrefresh(plw->_main_window);
  player_window_draw_inner(plw);
}

