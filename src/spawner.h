#ifndef FILE_SPAWNER_SEEN
#define FILE_SPAWNER_SEEN
/*
  deals with mapping string names to object spawners, which allow more complicated things to be easily embedded in map files, and also can be used to keep track of a spawning "stock", which means it can run out and stop spawning eventually
 */
typedef struct compound_spawner_struct compound_spawner;
typedef struct spawner_set_struct spawner_set;

#include "compound.h"

spawner_set* create_spawner_set(int len);
void free_spanwer_set(spawner_set* rm);
void spawner_set_append(spawner_set* ss, char* name, compound* (*func)(void));
void get_func_from_name(spawner_set* ss, char* name, compound* (**func)(void));
void get_func_from_idx(spawner_set* ss, int i, compound* (**func)(void));
char* get_name_from_idx(spawner_set* ss, int i);
int get_spawner_set_len(spawner_set* ss);

compound_spawner* create_compound_spawner(char* name, int cap, int x_pos, int y_pos);
void free_compound_spawner(compound_spawner* rm);

char* get_spawner_name(compound_spawner* spawn);
int get_spawner_cap(compound_spawner* spawn);
void get_spawner_pos(compound_spawner* spawn, virt_pos* result);

void trigger_spawners_in_map(map* map);
compound* compound_from_spawner(compound_spawner* spawn);
void trigger_spawner(compound_spawner* spawn, plane* insert);
void refund_spawner(compound_spawner* spawn);

#endif
