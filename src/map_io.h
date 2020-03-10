#ifndef FILE_MAP_IO_SEEN
#define FILE_MAP_IO_SEEN
#include "map.h"

map* load_map(char* filename);

void save_map(map* m, char* filename);

#endif 
