#ifndef FILE_MYOBJECTS_SEEN
#define FILE_MYOBJECTS_SEEN

typedef struct compound_spawner_struct compound_spawner;

#include "shapes.h"
#include "parts.h"
#include "map.h"
#include "plane.h"

#define TRASHCAN_SPAWN "trashcan_spawn"
#define BLUE_SLIME_SPAWN "blue_slime_spawn"
#define TEST_SPAWN "test_spawn"
#define GOHEI_SPAWN "gohei_spawn"
#define CEILING_GRASS_SHORT "ceil_grass_short"
#define FLOOR_GRASS_SHORT "floor_grass_short"
#define SPAWN_END "spawn_end"
extern char* spawn_array[];


char* get_spawner_name(compound_spawner* spawn);
int get_spawner_cap(compound_spawner* spawn);
void get_spawner_pos(compound_spawner* spawn, virt_pos* result);
compound_spawner* create_compound_spawner(char* name, int cap, int x_pos, int y_pos);
void free_compound_spawner(compound_spawner* rm);
void trigger_spawners_in_map(map* map);
compound* compound_from_spawner(compound_spawner* spawn);
void trigger_spawner(compound_spawner* spawn, plane* insert);
void refund_spawner(compound_spawner* spawn);

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
