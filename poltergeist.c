#include "poltergeist.h"
#include "input.h"
#include <assert.h>
//so, library is responsible for moving things
//kind of general, want movement to handle projectile, simple geometric movements, as well as playable and npc character movement
//the npc deal will probably require to backup querying system to get context of npc to other things in map
//otherwise, with my current movement system, just want poltergeists to fill in some movement and rotation fields
//also probably need body and map as in input, in anticipation of needs of querying system
//


poltergeist* make_poltergeist() {
  poltergeist* new = malloc(sizeof(poltergeist));
  new->posession = NULL;
  return new;
}

void user_poltergeist(spatial_hash_map* map,  struct body_struct* body, virt_pos* t_disp, double* r_disp) {
  polygon* poly = body->coll->shape;
  get_input_for_polygon(poly, t_disp, r_disp);
}

static int USER_POLTERGEIST = 0;

void give_user_poltergeist(poltergeist* polt) {
  if (USER_POLTERGEIST == 0) {
    polt->posession = &user_poltergeist;
    USER_POLTERGEIST = 1;
  }
  else {
    assert(0 && "created 2 user poltergeists\n");
  }
}


void apply_poltergeist(poltergeist* polt, spatial_hash_map* map,  struct body_struct* body, virt_pos* t_disp, double* r_disp) {
  if (polt != NULL) {
    polt->posession( map, body, t_disp, r_disp);
  }
  else {
    //literally do nothing
  }
}
