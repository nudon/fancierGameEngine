#include <assert.h>
#include "poltergeist.h"
#include "graphics.h"
#include "input.h"
#include "geometry.h"
#include "util.h"
//so, library is responsible for moving things
//kind of general, want movement to handle projectile, simple geometric movements, as well as playable and npc character movement


struct poltergeist_struct {
  void (*posession) (struct body_struct* body, vector_2* t_disp, double* r_disp);
  //might also have map/body as fields, 
};


#define POLT_LIM 10
char* polt_names[POLT_LIM];
void (*polt_funcs[POLT_LIM])(struct body_struct* body, vector_2* t_disp, double* r_disp);

void init_poltergeists() {
  null_init_array((void**)polt_names, POLT_LIM);
  null_init_array((void**)polt_funcs, POLT_LIM);
  int i = first_empty_index(polt_names, POLT_LIM);
  polt_names[i] = "no_polt";
  polt_funcs[i] = no_poltergeist;
  i = first_empty_index(polt_names, POLT_LIM);
  polt_names[i] = "user_polt";
  polt_funcs[i] = user_poltergeist;
  i = first_empty_index(polt_names, POLT_LIM);
  polt_names[i] = "standard_polt";
  polt_funcs[i] = standard_poltergeist;
  
  
  
}

char* null_text = "NULL";
char* get_polt_name(poltergeist* p) {
  if (p == NULL) {
    return null_text;
  }
  char* ret = "";
  for (int i = 0; i < POLT_LIM; i++) {
    if (p->posession == polt_funcs[i]) {
      ret = polt_names[i];
    }
  }
  return ret;
}

void set_polt_by_name(poltergeist* p, char* polt) {
  int i = char_search(polt_names, polt, POLT_LIM);
  if (0 <= i && i < POLT_LIM) {
    p->posession = polt_funcs[i];
  }
}



poltergeist* make_poltergeist() {
  poltergeist* new = malloc(sizeof(poltergeist));
  new->posession = no_poltergeist;
  return new;
}

void give_standard_poltergeist(poltergeist* polt) {
  polt->posession = &standard_poltergeist;
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


void apply_poltergeist(poltergeist* polt, struct body_struct* body, vector_2* t_disp, double* r_disp) {
  if (polt != NULL) {
    polt->posession(body, t_disp, r_disp);
  }
}

void no_poltergeist(struct body_struct* body, vector_2* t_disp, double* r_disp) {
  *t_disp = *zero_vec;
  *r_disp = 0;
}


void user_poltergeist(body* user_body, vector_2* t_disp, double* r_disp) {
  static int cam_init = 0;
  if (!cam_init) {
    set_camera_center(getCam(), getCenter(user_body));
    cam_init = 1;
  }
  polygon* poly = user_body->coll->shape;
  get_input_for_polygon(poly, t_disp, r_disp);
}
//given a compound and dir, move all bodies in that dir
//should be adding a force
//also seems like having a decaying average of a direction would be nice
//would add the soft behavior of things gradually aquiring new directions
//also might want to have some limited speed modes by modifying magnitude of direction. could either add hard-coded tiers/thresholds or take a log of magnitude for nice but sometimes funny stuff

void standard_poltergeist(struct body_struct* body, vector_2* t_disp, double* r_disp) {
  //move roughly in direcion of compounds dir
  
  vector_2 dir = get_curr_dir(get_gi(get_owner(body)));
  //might also take scales from gi
  double t_scale = 1.5;
  double r_scale = 0.004;
  double mag = log(vector_2_magnitude(&dir));
  if (mag > 0) {
    vector_2_scale(&dir, t_scale * mag , &dir);
    vector_2_add(t_disp, &dir, t_disp);
    r_scale *= mag;
  }
  //also reorient so body is "facing" dir
  double dir_theta = atan2(dir.v2, dir.v1);
  if (dir_theta < 0) {
    dir_theta += 2 * M_PI;
  }
  double diff = dir_theta - get_rotation(get_polygon(get_collider(body)));

  if (abs(diff) > M_PI) {
    if (diff > 0) {
      //fprintf(stderr, "1diff is %f\n", diff);
      diff = diff - 2 * M_PI;
    }
    else {
      //fprintf(stderr, "2diff is %f\n", diff);
      diff = diff + 2 * M_PI;
    }

  }
  //fprintf(stderr, "diff is %f\n", diff);
  *r_disp += r_scale * diff;
}
