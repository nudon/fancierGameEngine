#ifndef FILE_GI_SEEN
#define FILE_GI_SEEN

/*
  Sort of a holder for various fields and stats that are used by game intelligence, and also to keep track of things like health and jump state of objects

  Associated with bodies and compounds, though not every body or compound has a SMARTS associated with it
  
  Also, there is sort of 2 types of smarts, body smarts and compound smarts. body smarts hold memory and stat fields for a body, and point to an associated body and original compound owner. Compound smarts hold memory and stats fields for the whole compound, and points to a compound, but to no body

*/

typedef struct smarts_struct smarts;
typedef struct stam_struct stam;

typedef struct body_memory_struct body_memory;
typedef struct body_stats_struct body_stats;

typedef struct compound_stats_struct comp_stats;
typedef struct compound_memory_struct comp_memory;

#define SM_ATTACK "sm_attack"
#define SM_MOVE "sm_move"
#define SM_LOOK "sm_look"
#define SM_DANGER "sm_danger"
#define SM_USEFULL "sm_usefull"

#include "events.h"
#include "body.h"
#include "collider.h"
#include "attributes.h"



//creation of blank smarts
smarts * make_smarts();
//makes smart into a body smart
void smarts_body_init(smarts* sm);
//makes smart into a compound smart
void smarts_comp_init(smarts* sm);

//for sm, depending on if it's a body or comp sm, updates memory and stamina 
void update_smarts(smarts* sm);
//creates a body smarts and associates it with b
void add_smarts_to_body(body* b);
//creates a comp smarts and associates it with c
void add_smarts_to_comp(compound* c);
//frees smarts within compound
void free_body_smarts(body* b);
void free_compound_smarts(compound* c);

//calls damage_body if body stats make sense (not invulnerable, and the other body is danger. both bodies can be damaged)
void contact_damage(body* b1, body* b2);
//computes exact damage vals, based on scalars
//applies it do body, then also applies run-off damage to compound
void damage_body(body* b, double amt);

//sets contact damage in b stats and sets danger field in atts to 1, or 0 if val = 0
void set_contact_damage(body* b, int val);

//getters and setters, though the setter copies the bit-field value of atts
att* get_body_attributes(smarts* sm);
void set_body_attributes(smarts* sm, att* atts);

//compound stuff

//merely takes amt from c's health, or sets to zero if dead
void damage_compound(compound* c, double amt);
att* get_comp_attributes(smarts* sm);
void set_comp_attributes(smarts* sm, att* atts);
//vector_2 get_comp_movement(smarts* sm);


char* smarts_to_text(smarts* sm);
smarts*text_to_smarts(char* text);

//returns body/compound associated with sm
compound* get_smarts_compound(smarts* sm);
body* get_smarts_body(smarts* sm);

//adds a vector to some memory field, designated by tag
void add_to_smarts(smarts* sm, char* tag, vector_2* add);
//returns memory field value associated with tag
vector_2 get_from_smarts(smarts* sm, char* tag);

comp_stats* smarts_get_comp_stats(smarts* sm);
body_stats* smarts_get_body_stats(smarts* sm);


#endif


