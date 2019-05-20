#ifndef FILE_PLANE_SEEN
#define FILE_PLANE_SEEN

#include "collider.h"
#include "compound.h"

typedef struct {
  spatial_hash_map* map;
  gen_list* compounds_in_plane;
 
} plane;

plane* create_plane(spatial_hash_map* empty_map);

void add_compound_to_plane(plane* plane, compound* comp);

#endif
