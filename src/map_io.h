#ifndef FILE_MAP_IO_SEEN
#define FILE_MAP_IO_SEEN
#include "map.h"

map* load_map(char* filename);

void xml_write_map(FILE* file_out, map* map);

#endif 
