#ifndef FILE_CREATIONS_SEEN
#define FILE_CREATIONS_SEEN
/*
  holds some attempts at making static maps purely with code. Created several similar files for other pre-created objects, like objects, shapes
  also uses rooms which tries to ease the process of statically coding maps
*/


//typedef struct compound_spawner_struct compound_spawner; 

#include "compound.h"
#include "map.h"

//base creation maps
map* load_origin_map();
map* make_origin_map();
map* make_beach_map();
map* make_basic_map();

//creates, than writes base maps to disk
void write_maps_to_disk();

void make_compound_builder(compound* comp);

#endif
