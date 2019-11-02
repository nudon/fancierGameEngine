#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUND_SEEN

typedef struct compound_struct compound;

#include "body.h"
#include "myList.h"
#include "attributes.h"
#include "gi.h"

compound* create_compound();
body* get_compound_head(compound* c);
compound* mono_compound(body* b);

int move_body(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp);
void move_compound(spatial_hash_map* map, compound* c);

gen_list* get_bodies(compound* comp);

void add_body_to_compound(compound* comp, body* body);

void tether_join_compound(compound* comp, tether* teth);
void add_tether_to_compound(compound* comp, tether* teth);
gen_list* get_compound_tethers(compound* comp);

void set_compound_position(compound* comp, virt_pos* np);

void make_compound_uniform(compound* c);
int is_compound_uniform(compound* c);
int get_compound_uniform_flag(compound* c);
void set_compound_uniform_flag(compound* c, int v);

vector_2 get_dir(compound* comp);

void set_compound_gravity(compound* c, vector_2* grav);

void set_compound_smarts(compound* c, smarts* sm);
smarts* get_compound_smarts(compound * c);

#endif
