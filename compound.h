#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUND_SEEN

struct compound_struct;

#include "body.h"
#include "myList.h"
#include "attributes.h"
//okay, compound time
//will probably need a compound pointer in bodies as well

typedef struct compound_struct compound;

compound* create_compound();

decision_att* get_attributes(compound* comp);
void set_attributes(compound* comp, decision_att* na);

gen_list* get_bodies(compound* comp);

void add_body_to_compound(compound* comp, struct body_struct* body);

void tether_join_compound(compound* comp, struct tether_struct* teth, gen_list* append);

void set_compound_position(compound* comp, virt_pos* np);

vector_2 get_dir(compound* comp);
void set_dir(compound* comp, vector_2* nv);


void set_valid(compound* comp, int val);
int is_valid(compound* comp);
int get_valid(compound* comp);


#endif
