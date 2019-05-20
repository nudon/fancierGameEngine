#include <stdlib.h>
#include "plane.h"

plane* create_plane(spatial_hash_map* empty_map) {
  plane* new = malloc(sizeof(plane));
  new->map = empty_map;
  new->compounds_in_plane = createGen_list();
}


void add_compound_to_plane(plane* plane, compound* comp) {
  prependToGen_list(plane->compounds_in_plane, createGen_node(comp));
  //have to initialize collider
  insert_compound_in_shm(plane->map, comp);
}
