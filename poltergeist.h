#ifndef FILE_POLTERGEIST_SEEN
#define FILE_POLTERGEIST_SEEN
#include "collider.h"
#include "body.h"

//so, library is responsible for moving things
//kind of general, want movement to handle projectile, simple geometric movements, as well as playable and npc character movement
//the npc deal will probably require to backup querying system to get context of npc to other things in map
//otherwise, with my current movement system, just want poltergeists to fill in some movement and rotation fields
//also probably need body and map as in input, in anticipation of needs of querying system
//

typedef
struct poltergeist_struct {
  void (*posession) (struct spatial_hash_map_struct* map,  struct body_struct* body, vector_2* t_disp, double* r_disp);
  //might also have map/body as fields, 
} poltergeist;


poltergeist* make_poltergeist();

void user_poltergeist(struct spatial_hash_map_struct* map,  struct body_struct* body, vector_2* t_disp, double* r_disp);


void give_user_poltergeist(poltergeist* polt);


void apply_poltergeist(poltergeist* polt, struct spatial_hash_map_struct* map, struct body_struct* body, vector_2* t_disp, double* r_disp);
#endif
