#include "configuration.h"
#include "debug_window.h"
#include "engine.h"
#include "game_message_window.h"
#include "inventory_window.h"
#include "logger.h"
#include "ui/map_window.h"
#include "ui/player_window.h"
#include "ui_point.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

WINDOW        *init_ncurses();
Engine        *init_game(Configuration *);
Configuration *init_configuration();

#define PANIC(s)            \
  fprintf(stderr, "%s", s); \
  return -1;

int main(int argc, char *argv[]) {
  Configuration *configuration = init_configuration();
  const char    *error_message = "Critical error occured, check logs.\n";
  Engine        *engine = init_game(configuration);
  init_ncurses();
  logger_new(configuration_get_log_output_file(configuration), configuration_get_log_level(configuration));

  srandom(time(nullptr));

  LOG_INFO("Beginning game", 0);

  // The MapWindow should cover most of the height of the screen and a third of it
  MapWindow *map_window = map_window_new(engine_get_map(engine), LINES - 20, COLS / 3, 0, 0);
  if (map_window == nullptr) {
    PANIC(error_message);
  }

  // The Player Window is on right of the map window, it takes some height (8 lines) and 40 columns
  PlayerWindow *player_window = player_window_new(engine_get_active_entity(engine), 8, 40, map_window_get_cols(map_window) - 1, 0);
  if (player_window == nullptr) {
    PANIC(error_message);
  }

  InventoryWindow *inventory_window = inventory_window_new(engine_get_active_entity(engine), 10, player_window_get_cols(player_window),
                                                           map_window_get_cols(map_window) - 1, player_window_get_lines(player_window) - 1);
  if (inventory_window == nullptr) {
    PANIC(error_message);
  }

  GameMessageWindow *message_window = game_message_window_new(8, map_window_get_cols(map_window), map_window_get_lines(map_window) - 1, 0);
  if (message_window == nullptr) {
    PANIC(error_message);
  }

  UiPoint *ivw_ui_point = inventory_window_get_ui_point(inventory_window);

  int dbg_lines = 1 + LINES - (ui_point_get_y(ivw_ui_point) + inventory_window_get_lines(inventory_window));

  DebugWindow *debug_window = debug_window_new(engine, dbg_lines, inventory_window_get_cols(inventory_window),
                                               inventory_window_get_lines(inventory_window) + player_window_get_lines(player_window) - 2,
                                               map_window_get_cols(map_window) - 1);
  if (debug_window == nullptr) {
    PANIC(error_message);
  }

  debug_window_hide(debug_window);

  map_window_draw(map_window);
  player_window_draw(player_window);
  inventory_window_draw(inventory_window);
  game_message_window_draw(message_window);
  debug_window_draw(debug_window);

  doupdate();

  char key;

  while ((key = getch()) != 'q') {
    switch (key) {
      case 's':
        game_message_window_show_message(message_window, "Hey, that's an S!");
        break;
      case 'd':
        {
          if (debug_window_is_visible(debug_window)) {
            debug_window_hide(debug_window);
            game_message_window_show_message(message_window, "Hiding debug");
          } else {
            debug_window_show(debug_window);
            game_message_window_show_message(message_window, "Showing debug");
          }
        }
        break;
      default:
        engine_handle_keypress(engine, key);
        engine_move_all_entities(engine);
        break;
    }

    // Always draw AT THE END of all the operations
    map_window_draw(map_window);
    player_window_draw(player_window);
    inventory_window_draw(inventory_window);
    game_message_window_draw(message_window);
    debug_window_draw(debug_window);

    doupdate();
  }

  clear();

  engine_free(engine);
  configuration_free(configuration);
  logger_free(logger_instance());

  player_window_free(player_window);
  map_window_free(map_window);

  endwin();

  return 0;
}
