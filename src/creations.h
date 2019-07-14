#ifndef FILE_CREATIONS_SEEN
#define FILE_CREATIONS_SEEN

#include "compound.h"
#include "map.h"

compound* makeWalls();
compound* makeTriangle(virt_pos* cent);
compound* makeUserBody(virt_pos* cent);
compound* makeCentipede(int segments, gen_list* tethers, virt_pos* cent);
void make_compound_user(compound* comp);

body* makeBlock (int width, int height);
compound* makeBlockChain(virt_pos* start_pos, int len, int chain_type);
compound* makeBodyChain(body* start, virt_pos* start_pos,  int len, vector_2* dir, double disp);

event* make_load_event(virt_pos* cent);

map* make_origin_map();
map* make_room_map();
map* make_street_map();



map* load_origin_map();
map* load_room_map();
map* load_streets_map();
map* load_map_by_name(char* name);

void write_maps_to_disk();

#endif
