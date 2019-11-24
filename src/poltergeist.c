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
  i = first_empty_index(polt_names, POLT_LIM);
  polt_names[i] = "hand_polt";
  polt_funcs[i] = holder_poltergeist;
  
  
  
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
  else {
    fprintf(stderr, "error, unable to find poltergeist with name %s\n", polt);
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
  //virt_pos cent = get_body_center(user_body);
  polygon* p = get_polygon(get_collider(user_body));
  if (!cam_init) {
    set_camera_center(getCam(), read_only_polygon_center(p));
    cam_init = 1;
  }
  get_input_for_body(user_body, t_disp, r_disp);
}
//given a compound and dir, move all bodies in that dir
//should be adding a force
//also seems like having a decaying average of a direction would be nice
//would add the soft behavior of things gradually aquiring new directions
//also might want to have some limited speed modes by modifying magnitude of direction. could either add hard-coded tiers/thresholds or take a log of magnitude for nice but sometimes funny stuff

void standard_poltergeist(body* body, vector_2* t_disp, double* r_disp) {
  if (get_body_smarts(body) == NULL) {
    return;
  }
  vector_2 b_dir = get_smarts_movement(get_body_smarts(body));
  //vector_2 c_dir = get_smarts_movement(get_compound_smarts(get_owner(body)));
  vector_2 dir = b_dir;
  double t_scale = .0005;
  double r_scale = 0.004;
  double mag = vector_2_magnitude(&dir);
  if (mag > 0) {
    vector_2_scale(&dir, t_scale / mag , &dir);
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
      diff = diff - 2 * M_PI;
    }
    else {
      diff = diff + 2 * M_PI;
    }

  }
  //fprintf(stderr, "diff is %f\n", diff);
  *r_disp += r_scale * diff;
}


//takes body, applys a constant force in direction of torsos velocity
//paired with a tether between body and torso, holds body in front of torso
void holder_poltergeist(body* b, vector_2* t_disp, double* r_disp) {
  compound* c = get_owner(b);
  smarts* c_sm = get_compound_smarts(c);
  body* head = get_compound_head(c);
  vector_2 dir = *zero_vec, offset = *zero_vec;
  double mag = 0, f = 0.2;
  double theta;
  vector_2 temp = *zero_vec;
  virt_pos b_cent, head_cent;
  fizzle* fizz = get_fizzle(b);
  fizzle* base_fizz = get_base_fizzle(b);
  b_cent = get_body_center(b);
  head_cent = get_body_center(head);
  vector_between_points(&head_cent, &b_cent, &offset);
  dir = get_smarts_movement(c_sm);
  theta = angle_of_vector(&offset);
  if (!isZeroVec(&dir)) {
    mag = vector_2_magnitude(&dir);
    vector_2_scale(&dir, f / mag, &dir);
    add_velocity(fizz, &dir);

    if (offset.v1 <= 0) {
      set_reflections(get_polygon(get_collider(b)), -1,1);
      offset.v1 *= -1;
      theta = angle_of_vector(&offset);
    }
    else {
      set_reflections(get_polygon(get_collider(b)), 1,1);
    }
    push_shared_reflections(b);
    set_rotation(get_polygon(get_collider(b)), theta);
    //print_vector(&dir);
  }
  //weird shit with tethers
  get_tether(base_fizz, &temp);
  set_tether(base_fizz, zero_vec);
  add_tether(fizz, &temp);
  //print_vector(&dir); 
  //print_vector(&temp); 
  //print_point(&head_cent);
  }
