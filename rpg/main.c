#include "configuration.h"
#include "engine.h"
#include "entity.h"
#include "logger.h"
#include "map.h"
#include "ui/map_window.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

WINDOW        *init_ncurses();
Engine        *init_game(Configuration *);
Configuration *init_configuration();

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
  srandom(time(nullptr));
  Configuration *configuration = init_configuration();
  init_ncurses();
  Engine *engine = init_game(configuration);
  logger_new(configuration_get_log_output_file(configuration), configuration_get_log_level(configuration));
  LOG_INFO("Beginning game", 0);

  // The MapWindow should cover the whole height of the screen and a third of it
  MapWindow *map_window = map_window_new(engine_get_map(engine), LINES, COLS / 3, 3, 3);
  map_window_draw(map_window);

  // Print HP of the player
  MapBoundaries boundaries = map_get_boundaries(engine_get_map(engine));
  WINDOW       *w_player_info = newwin(4, 30, (boundaries.y / 2) - 2, boundaries.x + 5);

  print_player_info(engine, w_player_info);
  char key;

  while ((key = getch()) != 'q') {
    engine_handle_keypress(engine, key);
    engine_move_all_entities(engine);
    map_window_draw(map_window);
    print_player_info(engine, w_player_info);
  }

  clear();

  engine_free(engine);
  configuration_free(configuration);
  logger_free(logger_instance());
  endwin();

  return 0;
}
