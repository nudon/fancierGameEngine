#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUND_SEEN

typedef struct compound_struct compound;

#include "body.h"
#include "myList.h"
#include "attributes.h"
#include "gi.h"



compound* create_compound();

decision_att* get_attributes(compound* comp);
void set_attributes(compound* comp, decision_att* na);
gi* get_gi(compound* comp);
void set_gi(compound* comp, gi* g);

int body_update(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp);
void compound_update(spatial_hash_map* map, compound* c);

gen_list* get_bodies(compound* comp);

void add_body_to_compound(compound* comp, struct body_struct* body);

void tether_join_compound(compound* comp, struct tether_struct* teth, gen_list* append);

void set_compound_position(compound* comp, virt_pos* np);

void make_compound_uniform(compound* c);
int is_compound_uniform(compound* c);
int get_compound_uniform_flag(compound* c);
void set_compound_uniform_flag(compound* c, int v);


#endif
