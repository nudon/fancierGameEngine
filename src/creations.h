#ifndef FILE_CREATIONS_SEEN
#define FILE_CREATIONS_SEEN

typedef struct compound_spawner_struct compound_spawner; 

#include "compound.h"
#include "map.h"


char* get_spawner_name(compound_spawner* spawn);
int get_spawner_cap(compound_spawner* spawn);
void get_spawner_pos(compound_spawner* spawn, virt_pos* result);
compound_spawner* create_compound_spawner(char* name, int cap, int x_pos, int y_pos);
void trigger_spawners_in_map(map* map);
void trigger_spawner(compound_spawner* spawn, plane* insert);

compound* makeWalls();
void make_compound_user(compound* comp);

body* makeBlock (int width, int height);
compound* makeBlockChain(int pos_x, int pos_y, int width, int height, char* pic_fn, int len, int chain_type);
compound* makeBodyChain(body* start, virt_pos* start_pos,  int len, vector_2* dir, double disp);

void insert_load_zone_into_plane(char* from_map, char* to_map, plane* from_plane, char* to_plane_name, virt_pos* from_pos, virt_pos* to_pos);
event* make_load_event(virt_pos* cent);

map* make_origin_map();
map* make_room_map();
map* make_street_map();



map* load_origin_map();
map* load_room_map();
map* load_streets_map();
map* make_beach_map();
map* load_map_by_name(char* name);

void write_maps_to_disk();

//headers for compound spawners
compound* makeCrab(int pos_x, int pos_y);
compound* makeSlime(int pos_x, int pos_y);
compound* tunctish(int pos_x, int pos_y);
compound* makeTrashCan(int pos_x, int pos_y);

#endif
