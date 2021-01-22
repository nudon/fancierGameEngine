#ifndef FILE_MAP_SEEN
#define FILE_MAP_SEEN

/*
  keeps track of a map, which is a stacked list of planes 

  also holds some load-zone definitions, which is like a warp zone that can lead to point in a plane in a map, which can be the same plane/map or a different plane/map 
  
  also holds travle lists, which keeps track of objects going through load zones, sort of a intermediate holding area to stay in while being taken out of one map/plane and put in another

  keeps a travel list, a 2D array indexed by [mapname][planename] that returns a list, which stores things that are traveling to that plane in that map
and some other arrays for converting(or finding) indexes for a name
also constants for max dim of said 2D array
*/

typedef struct map_struct map;
typedef struct load_zone_struct load_zone;

#include <stdlib.h>
#include "events.h"
#include "myList.h"
#include "plane.h"

//creates an empty map
map* create_map(char* name);
//gettters and setters
gen_list* get_planes(map* map);
plane* get_plane_by_name(map* m, char* name);
char* get_map_name(map* m);
void set_map_name(map* m, char* name);

//appends a plane to the map
void add_plane(map* map, plane* plane);

//load_zone functions
//null_inits first layers of internal arrays
void init_map_load();

//creates travel list for each load_zone in each plane in map
void map_load_create_travel_lists(map* map);

//creates a load_zone with given args
load_zone* make_load_zone(char* m_from, char* m_to, char* p_from, char* p_to, virt_pos* dest, event* trigger);

//getters for lz
char* get_lz_from_map(load_zone* lz);
char* get_lz_to_map(load_zone* lz);
char* get_lz_from_plane(load_zone* lz);
char* get_lz_to_plane(load_zone* lz);

virt_pos get_lz_dest(load_zone* lz);
event* get_lz_event(load_zone* lz);

//runs if something steps on a load zone..if compound has travle flag set, then it removes it from current map and places it in travle list. If compound is a user, then it triggers a map change
int trigger_map_change(load_zone* lz, compound* trav);
//goes through each load zone in each plane in each map, and calls trigger_map_change for each compound that steps into a load zone
void check_load_triggers(map* map);

//steps to take before saving a map, currently just refunding spawners
void prep_for_save(map* m);
//steps to take before loading a map, currently triggering spawners and flushing travle lists for map
void prep_for_load(map* m);
//since spawners can have a stock, compounds which are still there when a map is about to be saved, give back a stock if they have a spawner, then are removed from the map
void refund_spawners_in_map(map* m);

#endif
