#include <stdlib.h>
#include "plane.h"
#include "creations.h"


struct plane_struct {
  spatial_hash_map* map;
  gen_list* compounds_in_plane;
  gen_list* tethers_in_plane;
  gen_list* events_in_plane;
  gen_list* load_zones_in_plane;
  gen_list* spawners_in_plane;
  double z_level;
  char* plane_name;
};


plane* create_plane(spatial_hash_map* empty_map, char* name) {
  plane* new = malloc(sizeof(plane));
  new->map = empty_map;
  new->compounds_in_plane = create_gen_list();
  new->tethers_in_plane = create_gen_list();
  new->events_in_plane = create_gen_list();
  new->load_zones_in_plane = create_gen_list();
  new->spawners_in_plane = create_gen_list();
  new->z_level = 1;
  new->plane_name = strdup(name);
  return new;
}

spatial_hash_map* get_shm(plane* plane) {
  return plane->map;
}

gen_list* get_compounds(plane* plane) {
  return plane->compounds_in_plane;
}

gen_list* get_tethers(plane* plane) {
  return plane->tethers_in_plane;
}

gen_list* get_events(plane* plane) {
  return plane->events_in_plane;
}

gen_list* get_load_zones(plane* plane) {
  return plane->load_zones_in_plane;
}

gen_list* get_spawners(plane* plane) {
  return plane->spawners_in_plane;
}

char* get_plane_name(plane* p) { return p->plane_name; }

double get_z_level(plane* plane) {
  return plane->z_level;
}

void set_z_level(plane* plane, double nz) {
  plane->z_level = nz;
}

void add_compound_to_plane(plane* plane, compound* comp) {
  list_prepend(get_compounds(plane), create_gen_node(comp));
  insert_compound_in_shm(plane->map, comp);
}

//returns 1 if comp was present and removed, 0 if not
int remove_compound_from_plane(plane* plane, compound* comp) {
  gen_node* curr = get_compounds(plane)->start;
  int ret = 0;
  while(curr != NULL) {
    if ((compound*)curr->stored == comp) {
      remove_compound_from_shm(get_shm(plane), comp);
      remove_node(curr);
      free(curr);
      curr = NULL;
      ret = 1;
    }
    else {
      curr = curr->next;
    }
  }
  return ret;
}

void add_tether_to_plane(plane* plane, tether* teth) {
  list_prepend(get_tethers(plane), create_gen_node(teth));
}

void add_event_to_plane(plane* plane, event* event) {
  list_prepend(get_events(plane), create_gen_node(event));
}

void add_load_zone_to_plane(plane* plane, load_zone* lz) {
  list_prepend(get_load_zones(plane), create_gen_node(lz));
}

void add_spawner_to_plane(plane* plane, compound_spawner* spawn) {
  list_prepend(get_spawners(plane), create_gen_node(spawn));
}

