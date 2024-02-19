#include "configuration.h"
#include "engine.h"
#include "entity.h"
#include "logger.h"
#include "ui/map_window.h"
#include "ui/player_window.h"
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

void print_player_info(Engine *engine, WINDOW *window) {
  if (engine_has_active_entity(engine)) {
    wmove(window, 0, 0);
    wprintw(window, "  HP: %d", entity_get_life_points(engine_get_active_entity(engine)));
    wmove(window, 1, 0);
    wprintw(window, "NAME: %s", entity_get_name(engine_get_active_entity(engine)));
    wrefresh(window);
  }
}

int main(int argc, char *argv[]) {
  const char *error_message = "Critical error occured, check logs.\n";
  srandom(time(nullptr));
  Configuration *configuration = init_configuration();
  init_ncurses();
  Engine *engine = init_game(configuration);
  logger_new(configuration_get_log_output_file(configuration), configuration_get_log_level(configuration));
  LOG_INFO("Beginning game", 0);

  // The MapWindow should cover most of the height of the screen and a third of it
  MapWindow *map_window = map_window_new(engine_get_map(engine), LINES - 5, COLS / 3, 0, 0);
  if (map_window == nullptr) {
    PANIC(error_message);
  }

  // The Player Window is on right of the map window, it takes some height (8 lines) and 40 columns
  PlayerWindow *player_window = player_window_new(engine_get_active_entity(engine), 8, 40, map_window_get_cols(map_window) - 1, 0);
  if (player_window == nullptr) {
    PANIC(error_message);
  }

  map_window_draw(map_window);
  player_window_draw(player_window);

  char key;

  while ((key = getch()) != 'q') {
    engine_handle_keypress(engine, key);
    engine_move_all_entities(engine);

    // Always draw AT THE END of all the operations
    map_window_draw(map_window);
    player_window_draw(player_window);
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
