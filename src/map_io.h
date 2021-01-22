#ifndef FILE_MAP_IO_SEEN
#define FILE_MAP_IO_SEEN
#include "map.h"
/*
  defines how to write maps to a file, and how to read that file back into a map. 

  basic structures are generally directly serialized in the map, but things that hold smarts or tethers are generally represented instead as a spawner with a specific name
 */


map* load_map(char* filename);

void save_map(map* m, char* filename);

#endif 
