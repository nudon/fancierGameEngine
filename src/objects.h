#ifndef FILE_MYOBJECTS_SEEN
#define FILE_MYOBJECTS_SEEN
/*
  one of the pre-created object files, generally for special types of compounds or creatures. 
 */

#include "shapes.h"
#include "parts.h"
#include "map.h"
#include "plane.h"
#include "spawner.h"

#define TRASHCAN_SPAWN "trashcan_spawn"
#define BLUE_SLIME_SPAWN "blue_slime_spawn"
#define TEST_SPAWN "test_spawn"
#define GOHEI_SPAWN "gohei_spawn"
#define CEILING_GRASS_SHORT "ceil_grass_short"
#define FLOOR_GRASS_SHORT "floor_grass_short"
#define SPAWN_END "spawn_end"
extern spawner_set* main_set;

void init_spawn_set();

compound* makeCentipede(int segments);
compound* roper(int segments);
compound* makeCrab();
compound* makeSlime();
compound* tunctish();
compound* monkey();
compound* makeTrashCan();
compound* makeGohei();
compound* ceiling_grass_short();
compound* floor_grass_short();

#endif
