#ifndef FILE_MYBODY_SEEN
#define FILE_MYBODY_SEEN

/*
  Bodies combine collidable things with a physics attribute, and are the basic objects in the game. Can be animate/inanimate, controlled by the user
*/



typedef struct body_struct body;
typedef struct body_stats_struct body_stats;


#include "collider.h"
#include "physics.h"
#include "poltergeist.h"
#include "compound.h"
#include "graphics.h"
#include "events.h"
#include "gi.h"
#include "shared_input.h"

#define SI_CENTER -1

//creates and frees shared_input. need references for ?
shared_input* create_shared_input();
shared_input** create_shared_input_ref();
void free_shared_input(shared_input* rm);
void free_shared_input_ref(shared_input** rm);

//gets shared input
shared_input* get_shared_input(body* b);
shared_input** get_shared_input_ref(body* b);
//adds/removes shared input for body
void set_shared_input(body* b, shared_input** si);
void un_set_shared_input(body* b);


//creates/frees/clones body
body* createBody(fizzle* fizz, struct collider_struct* coll);
body* cloneBody(body* src);
void free_body(body* rm);



//getters/setters for fields
int get_move_status(body* b);
void set_move_status(body* b, int val);
fizzle* get_fizzle(body* body);
fizzle* get_base_fizzle(body* b);
collider * get_collider(body* body);
compound* get_owner(body* body);
void set_owner(body* b, compound* o);
fizzle* get_fizzle(body* aBody);
virt_pos get_body_center(body* b);
void offset_body(body* b, virt_pos* offset);
void set_body_gravity(body* b, vector_2* grav);
void set_body_center(body* b, virt_pos* vp);
picture* get_picture(body* aBody);
void set_poltergeist(body* b, poltergeist* polt);
poltergeist* get_poltergeist(body* b);
void set_picture(body* aBody, picture* pic);
void set_picture_by_name(body* aBody, char* fn);

//adding/getting events
void add_event_to_body(body* b, event* e);
gen_list* get_body_events(body* b);

flags* body_get_draw_flags(body* b);

//creates a tether between two bodies
tether* tether_bodies(body* b1, body* b2, tether* tether_params);

//use body poltergeist to add linear/rotational velocities to body
void run_body_poltergeist(body* b);

//getter/setter for fields
void set_body_smarts(body* b, smarts* sm);
smarts* get_body_smarts(body* b);

//special operations for mirroring sprites, uniform via shared input
void push_shared_reflections(body* b);
void pull_shared_reflections(body* b);

//defined in text driver currently
int move_body(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp);

//returns if b2 and b1 have different owners(main compounds)
int foreign_body(body* b1, body* b2);

//returncs the vector that points from the center of b1 to the center of b2
vector_2 vector_between_bodies(body* b1, body* b2);

#endif
