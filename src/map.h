#ifndef FILE_MAP_SEEN
#define FILE_MAP_SEEN

typedef struct map_struct map;
typedef struct load_zone_struct load_zone;

#include <stdlib.h>
#include "events.h"
#include "myList.h"
#include "plane.h"

map* create_map(char* name);
gen_list* get_planes(map* map);
plane* get_plane_by_name(map* m, char* name);
void add_plane(map* map, plane* plane);
char* get_map_name(map* m);
map* getMap();
void setMap(map* new);


//load_zone functions
void init_map_load();

void map_load_create_travel_lists(map* map);

load_zone* make_load_zone(char* m_from, char* m_to, char* p_from, char* p_to, virt_pos* dest, event* trigger);

char* get_lz_from_map(load_zone* lz);
char* get_lz_to_map(load_zone* lz);
char* get_lz_from_plane(load_zone* lz);
char* get_lz_to_plane(load_zone* lz);

virt_pos get_lz_dest(load_zone* lz);
event* get_lz_event(load_zone* lz);

void trigger_map_change(load_zone* lz, compound* trav);
void check_load_triggers(map* map);

#endif
