#include <stdlib.h>
#include <stdio.h>
#include "gi.h"
#include "attributes.h"
#include "map.h"
//temp folder for game intelligence stuff

event* make_basic_vision_event(body* b) {
  polygon* event_area = vision_triangle(150, 200, 0);
  event* e = make_event(event_area);
  set_event(e, basic_decide_event);
  add_event_to_body(b,e);
  return e;  
}

//visual systmes

//making the cones of vision
//just make an isosolece triangle of base_width and height
//make an event out of it and attach to a body
polygon* vision_triangle(int base, int depth, double rot_off) {
  polygon* tri = createPolygon(3);
  virt_pos point = *zero_pos;
  set_base_point(tri, 0, &point);
  point = (virt_pos){.x = depth, .y = base / 2};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 1, &point);
  point = (virt_pos){.x = depth, .y = -base / 2};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 2, &point);
  generate_normals_for_polygon(tri);
  return tri;
}

struct gi_struct {
  vector_2 curr_dir;
  vector_2 new_dir;
  double exp_decay_alpha;
  decision_att* decision_attributes;
};

gi* create_gi() {
  gi* new = malloc(sizeof(gi));
  new->curr_dir = *zero_vec;
  new->new_dir = *zero_vec;
  new->exp_decay_alpha = 0.05;
  new->decision_attributes = make_decision_att();
  return new;
}

void free_gi(gi* g) {
  free_decision_att(g->decision_attributes);
  free(g);
}

void add_to_dir(gi* g, vector_2* dir) {
  vector_2_add(&(g->new_dir), dir, &(g->new_dir));
}

void calc_new_dir(gi* g) {
  exponential_decay_vector(&(g->curr_dir), &(g->new_dir), &(g->curr_dir), g->exp_decay_alpha);
  //issue with this
  //dir will always get set to 0,0 angle for rotation will also be zero
  //so things will all face to right of screen
  g->new_dir = *zero_vec;
}

vector_2 get_curr_dir(gi* g) {
  return g->curr_dir;
}

void set_curr_dir(gi* g, vector_2* vec) {
  g->curr_dir = *vec;
}

vector_2 get_new_dir(gi* g) {
  return g->new_dir;
}

void set_new_dir(gi* g, vector_2* vec) {
  g->new_dir = *vec;
}

decision_att* get_gi_attributes(gi* g) {
  return g->decision_attributes;
}

void set_gi_attributes(gi* g, decision_att* atts) {
  copy_atts(atts, get_gi_attributes(g));
}


double get_exp_decay_alpha(gi* g) {
  return g->exp_decay_alpha;
}

void set_exp_decay_alpha(gi* g, double val) {
  g->exp_decay_alpha = val;
}
