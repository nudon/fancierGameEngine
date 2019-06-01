#ifndef FILE_CREATIONS_SEEN
#define FILE_CREATIONS_SEEN

#include "compound.h"
#include "map.h"

compound* makeWalls();
compound* makeTriangle();
compound* makeUserBody();
compound* makeCentipede(int segments, gen_list* tethers);
void make_compound_user(compound* comp);
event* make_load_event(virt_pos* cent);

map* make_origin_map();
map* make_street_map();


map* load_origin_map();
map* load_streets_map();
map* load_map_by_name(char* name);

void write_maps_to_disk();

#endif
