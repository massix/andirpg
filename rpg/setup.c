#include "configuration.h"
#include "entity.h"
#include "map.h"
#include "tile.h"
#include <assert.h>
#include <engine.h>
#include <ncurses.h>
#include <stdint.h>

Configuration *init_configuration() {
  Configuration *ret = configuration_new("./andirpg.ini");
  configuration_init(ret);
  return ret;
}

WINDOW *init_ncurses() {
  WINDOW *main_window = initscr();

  refresh();
  noecho();
  cbreak();
  curs_set(0);
  start_color();
  use_default_colors();

  // Active player
  init_pair(1, COLOR_RED, -1);
  // Mountains
  init_pair(2, COLOR_BLUE, -1);
  // Inhumans
  init_pair(3, COLOR_MAGENTA, -1);
  // Trees
  init_pair(4, COLOR_GREEN, -1);
  // Animals
  init_pair(5, COLOR_YELLOW, -1);

  return main_window;
}

Engine *init_game(Configuration *configuration) {
  assert(configuration != nullptr);
  Engine *engine = engine_new(map_new(10, 10, 20, "Rimini Centro"));

  // The player
  engine_add_entity(
    engine, entity_build(configuration_get_player_starting_hp(configuration), HUMAN, configuration_get_player_name(configuration), 0, 0));

  // The enemy
  engine_add_entity(engine, entity_build(20, INHUMAN, "v1", 0, 7));

  // A small forest
  engine_add_entity(engine, entity_build(15, TREE, "t1", 7, 0));
  engine_add_entity(engine, entity_build(15, TREE, "t2", 8, 0));
  engine_add_entity(engine, entity_build(15, TREE, "t3", 9, 0));

  // Mountains
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m1", 0, 2));
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m2", 1, 2));
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m3", 2, 2));
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m4", 3, 2));
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m5", 4, 2));

  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m6", 1, 3));
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m7", 2, 3));
  engine_add_entity(engine, entity_build(30, MOUNTAIN, "m8", 3, 3));

  // A bigger forest
  engine_add_entity(engine, entity_build(30, TREE, "t4", 5, 6));
  engine_add_entity(engine, entity_build(30, TREE, "t5", 6, 6));
  engine_add_entity(engine, entity_build(30, TREE, "t6", 7, 6));
  engine_add_entity(engine, entity_build(30, TREE, "t7", 5, 7));
  engine_add_entity(engine, entity_build(30, TREE, "t8", 7, 7));
  engine_add_entity(engine, entity_build(30, TREE, "t9", 7, 8));

  // An animal
  engine_add_entity(engine, entity_build(30, ANIMAL, "a1", 6, 7));

  engine_set_active_entity(engine, configuration_get_player_name(configuration));

  TileProperties props;
  props.kind = FLOOR;
  props.inside = true;
  props.traversable = true;
  props.base_light = 0;

  // Set some tiles as unlit just for debug purposes
  for (uint32_t x = 6; x < 8; x++) {
    for (uint32_t y = 1; y < 5; y++) {
      props.base_light = props.base_light + 1 % 10;
      map_set_tile_properties(engine_get_map(engine), x, y, &props);
    }
  }

  return engine;
}

