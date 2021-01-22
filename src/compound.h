#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUND_SEEN
/*
  used for grouping compounds together conceptually, generally doing things like setting compound gravity or setting compound position applies to each body in the compound
  
  also, things in the same compound ignore collision
*/
typedef struct compound_struct compound;

#include "body.h"
#include "myList.h"
#include "attributes.h"
#include "objects.h"
#include "gi.h"

//creation and deletion
compound* create_compound();
void free_compound(compound* rm);

//returns first body in compound's body list
body* get_compound_head(compound* c);

//getter/setter for where a compound spawner is/spawns compounds
void set_spawner_p(compound* c, compound_spawner* cs);
compound_spawner* get_spawner_p(compound* c);

//puts a body into a single-body compound
compound* mono_compound(body* b);

//offsets bodies in compound
void offset_compound(compound* c, virt_pos* offset);

//returns list of bodies
gen_list* get_bodies(compound* comp);

//adds a body to compound's list
void add_body_to_compound(compound* comp, body* body);

//remove tethers within compound
void cut_compound(compound* c);

//sequentially joins body in compound with teth parameter,s or default tether
void tether_join_compound(compound* comp, tether* teth);

//adds a tether to compounds internal list
void add_tether_to_compound(compound* comp, tether* teth);
//returns said list
gen_list* get_compound_tethers(compound* comp);

//sets all bodies centers within compount to np
void set_compound_position(compound* comp, virt_pos* np);

//sets bodies in compount to have gravity of grav
void set_compound_gravity(compound* c, vector_2* grav);

//getters and setters
void set_compound_smarts(compound* c, smarts* sm);
smarts* get_compound_smarts(compound * c);


#endif
