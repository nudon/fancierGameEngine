#ifndef FILE_PLANE_SEEN
#define FILE_PLANE_SEEN

typedef struct plane_struct plane;

#include "collider.h"
#include "compound.h"
#include "physics.h"
#include "events.h"
#include "map.h"
#include "creations.h"



plane* create_plane(spatial_hash_map* empty_map, char* name);

spatial_hash_map* get_shm(plane* plane);
gen_list* get_compounds(plane* plane);
gen_list* get_tethers(plane* plane);
gen_list* get_events(plane* plane);
gen_list* get_load_zones(plane* plane);
gen_list* get_spawners(plane* plane);
char* get_plane_name(plane* p);
double get_z_level(plane* plane);
void set_z_level(plane* plane, double nz);

void add_compound_to_plane(plane* plane, compound* comp);
int remove_compound_from_plane(plane* plane, compound* comp);

void add_tether_to_plane(plane* plane, tether* teth);
void add_event_to_plane(plane* plane, event* event);
void add_load_zone_to_plane(plane* plane, load_zone* lz);
void add_spawner_to_plane(plane* plane, compound_spawner* spawn);
#endif
