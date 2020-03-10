#ifndef FILE_GI_SEEN
#define FILE_GI_SEEN

typedef struct smarts_struct smarts;
#define SM_ATTACK "sm_attack"
#define SM_MOVE "sm_move"
#define SM_LOOK "sm_look"
#define SM_DANGER "sm_danger"
#define SM_USEFULL "sm_usefull"

#include "events.h"
#include "body.h"
#include "collider.h"
#include "attributes.h"



//smarts stuff
smarts * make_smarts();
void smarts_body_init(smarts* sm);
void smarts_comp_init(smarts* sm);

void update_smarts(smarts* sm);
void add_smarts_to_body(body* b);
void add_smarts_to_comp(compound* b);
void free_body_smarts(body* b);
void free_compound_smarts(compound* c);

//body stuff
void contact_damage(body* b1, body* b2);
void set_contact_damage(body* b, int val);
void damage_body(body* b, double amt);
att* get_body_attributes(smarts* sm);
void set_body_attributes(smarts* sm, att* atts);

//compound stuff

void damage_compound(compound* c, double amt);
att* get_comp_attributes(smarts* sm);
void set_comp_attributes(smarts* sm, att* atts);
//vector_2 get_comp_movement(smarts* sm);


event* make_basic_vision_event(body* b);

polygon* vision_cone(int radius, double theta_deg, int steps, double rot_off);
polygon* vision_triangle(int base, int depth, double rot_off);


char* smarts_to_text(smarts* sm);
smarts*text_to_smarts(char* text);

compound* get_smarts_compound(smarts* sm);
body* get_smarts_body(smarts* sm);
int get_smarts_comp_max_health(smarts* sm);
int get_smarts_comp_health(smarts* sm);
int get_smarts_comp_max_jumps(smarts* sm);
int get_smarts_comp_jumps(smarts* sm);

void add_to_smarts(smarts* sm, char* tag, vector_2* add);
vector_2 get_from_smarts(smarts* sm, char* tag);

void jump_action(compound* c);
void end_jump(compound* c);
void jump_action_reset(compound* c);
void pickup_action(compound* c);
void throw_action(compound* c);



vector_2 vision_inv_distance_scale(vector_2* vec);
vector_2 vision_speed_scale(vector_2* vec, vector_2* velocity);
 
 

#endif


