#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUND_SEEN

typedef struct compound_struct compound;

#include "body.h"
#include "myList.h"
#include "attributes.h"
#include "objects.h"
#include "gi.h"

compound* create_compound();
body* get_compound_head(compound* c);
void set_spawner_p(compound* c, compound_spawner* cs);
compound_spawner* get_spawner_p(compound* c);
compound* mono_compound(body* b);

gen_list* get_bodies(compound* comp);

void add_body_to_compound(compound* comp, body* body);


void cut_compound(compound* c);
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

void free_compound(compound* rm);
#endif
