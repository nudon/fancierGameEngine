#ifndef FILE_CREATIONS_SEEN
#define FILE_CREATIONS_SEEN

typedef struct compound_spawner_struct compound_spawner; 

#include "compound.h"
#include "map.h"

void insert_load_zone_into_plane(char* from_map, char* to_map, plane* from_plane, char* to_plane_name, virt_pos* from_pos, virt_pos* to_pos);

map* load_origin_map();
map* make_origin_map();
map* make_beach_map();
map* make_basic_map();


void write_maps_to_disk();

void make_compound_builder(compound* comp);

#endif
