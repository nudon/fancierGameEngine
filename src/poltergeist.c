#include <assert.h>
#include "poltergeist.h"
#include "graphics.h"
#include "input.h"
#include "geometry.h"
#include "util.h"
#include "builder.h"
#include "guts.h"


struct poltergeist_struct {
  void (*posession) (struct body_struct* body, vector_2* t_disp, double* r_disp);
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
  polt_names[i] = "look_polt";
  polt_funcs[i] = look_poltergeist;
  i = first_empty_index(polt_names, POLT_LIM);
  polt_names[i] = "hand_polt";
  polt_funcs[i] = holder_poltergeist;
  i = first_empty_index(polt_names, POLT_LIM);
  polt_names[i] = "bb_polt";
  polt_funcs[i] = basic_brain;
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

static int BUILD_POLT = 0;

void give_builder_poltergeist(poltergeist* polt) {
  if (BUILD_POLT) {
    assert( 0 && "created 2 builders\n");
  }
  polt->posession = &builder_poltergeist;
  BUILD_POLT = 1;
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


void reorient(body* b, vector_2* vec, vector_2* t_disp, double* r_disp) {
  vector_2 dir = *vec;
  double r_scale = 0.04;
  //also reorient so body is "facing" dir
  double dir_theta = atan2(dir.v2, dir.v1);
  double body_theta = get_rotation(get_polygon(get_collider(b)));
  double diff = difference_of_radians(body_theta, dir_theta);
  *r_disp += r_scale * diff;
}

void translate(body* b, vector_2* vec, vector_2* t_disp, double* r_disp) {
  vector_2 dir = *vec;
  double t_scale = .2;
  double mag = vector_2_magnitude(&dir);
  if (mag > 0) {
    vector_2_scale(&dir, t_scale / mag , &dir);
    vector_2_add(t_disp, &dir, t_disp);
  }
}


