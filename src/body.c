#include "body.h"
#include <math.h>

body* createBody(fizzle* fizz, struct collider_struct* coll) {
  body* new = malloc(sizeof(body));
  new->fizz = fizz;
  new->coll = coll;
  new->polt = NULL;
  coll->body = new;
  new->owner = NULL;
  new->pic = NULL;
  new->event_list = createGen_list();
  new->status = 0;
  new->pic = make_picture(NULL);
  return new;
}

body* cloneBody(body* src) {
  body* new = malloc(sizeof(body));
  new->fizz = cloneFizzle(src->fizz);
  new->coll = cloneCollider(src->coll);
  new->polt = NULL;
  new->coll->body = new;
  new->owner = NULL;
  new->pic = src->pic;
  new->event_list = createGen_list();
  new->status = 0;
  return new;
}

void free_body(body* rm) {
  free_fizzle(rm->fizz);
  free_collider(rm->coll);
  free(rm);
}

collider * get_collider(body* body) {
  return body->coll;
}

compound* get_owner(body* body) {
  return body->owner;
}

fizzle* get_fizzle(body* aBody) {
  return aBody->fizz;
}

double get_mass(body* aBody) {
  return aBody->fizz->mass;
}

vector_2* get_velocity(body* aBody) {
  return &(aBody->fizz->velocity);
}

virt_pos* get_body_center(body* b) {
  return get_center(get_polygon(get_collider(b)));
}

void set_body_center(body* b, virt_pos* vp) {
  *(get_body_center(b)) = *vp;
}

picture* get_picture(body* aBody) {
  return aBody->pic;
}

void set_picture(body* aBody, picture* pic) {
  if (aBody->pic != NULL && strcmp(aBody->pic->file_name ,DEF_FN) != 0) {
    fprintf(stderr, "warning, overwriting a bodies picture\n");
  }
  aBody->pic = pic;
}

void set_poltergeist(body* b, poltergeist* polt) {
  b->polt = polt;
}
poltergeist* get_poltergeist(body* b) {
  return b->polt;
}

void set_picture_by_name(body* aBody, char* fn) {
  picture* pic = make_picture(fn);
  set_picture(aBody, pic);
}


void add_event_to_body(body* b, event* e) {
  prependToGen_list(b->event_list, createGen_node(e));
  set_event_body(e, b);
}

gen_list* get_body_events(body* b) {
  return b->event_list;
}


void resolve_collisions(spatial_hash_map* map, body* main_body) {
  collider* main_coll = main_body->coll;
  
  vector* occupied = main_coll->collider_node->active_cells;
  gen_list list;
  initGen_list(&list);
  store_unique_colliders_in_list(map, occupied, &list);
  gen_node* currNode = list.start;
  body* currBody;
  while(currNode != NULL) {
    currBody =((collider*)currNode->stored)->body;
    if (currBody != main_body && currBody->owner != main_body->owner) {
      resolve_collision(map, main_body, currBody);
    }
    currNode = currNode->next;
  }
  clean_collider_list(&list);

}

void resolve_collision(spatial_hash_map* map, body* body1, body* body2) {
  polygon* p1 = body1->coll->shape;
  polygon* p2 = body2->coll->shape;
  vector_2 normal_of_collision = *zero_vec;
  vector_2 b1_norm = *zero_vec;
  vector_2 b2_norm = *zero_vec;
  
  int actual_collision = find_mtv_of_polygons(p1, p2, &normal_of_collision);
  double mtv_mag = vector_2_magnitude(&normal_of_collision);
  //potentially theres' no actual collision since everything has been coarse grain at this point
  if (actual_collision != 0) {
    make_unit_vector(&normal_of_collision, &normal_of_collision);
    get_normals_of_collision(body1, body2, &normal_of_collision, &b1_norm, &b2_norm);
    
    displace_bodies(map,body1, body2, mtv_mag, &b1_norm, &b2_norm);
    impact(body1, body2, &b1_norm);
  }
}

void inv_mass_contribution(double m1, double m2, double* m1c, double* m2c) {
  mass_contribution(m1, m2, m2c, m1c);
}

void mass_contribution(double m1, double m2, double* m1c, double* m2c) {
  double sum;
  if (isinf(m1) && isinf(m2)) {
    *m1c = 0;
    *m2c = 0;
  }
  else if (isinf(m1) || isinf(m2)) {
    *m1c = 1;
    *m2c = 0;
    if (isinf(m2)) {
    *m1c = 0;
    *m2c = 1;
    }
  }
  else {
    sum = m1 + m2;
    *m1c = m1 / sum;
    *m2c = m2 / sum;
  }
}

void displace_bodies(spatial_hash_map* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit) {

  vector_2 b1tv = *b1tv_unit;
  vector_2 b2tv = *b2tv_unit;
  
  virt_pos b1d = *zero_pos;
  virt_pos b2d = *zero_pos;
  double b1Scale = 1;
  double b2Scale = 1;
  double b1Mass = get_mass(b1);
  double b2Mass = get_mass(b2);

  inv_mass_contribution(b1Mass, b2Mass, &b1Scale, &b2Scale);

  b1Scale *= mtv_mag;
  b2Scale *= mtv_mag;
  vector_2_scale(&b1tv, b1Scale, &b1tv);
  vector_2_scale(&b2tv, b2Scale, &b2tv);  

  
  vector_2_to_virt_pos(&b1tv, &b1d);
  vector_2_to_virt_pos(&b2tv, &b2d);

  body_update(map, b1, &b1d, 0.0);
  body_update(map, b2, &b2d, 0.0);
  
}


void get_normals_of_collision(body* body1, body* body2, vector_2* normal, vector_2* body1_norm, vector_2* body2_norm) {
  double l1, l2;
  l1 = get_projected_length_pos(get_body_center(body1), normal);
  l2 = get_projected_length_pos(get_body_center(body2), normal);
  *body1_norm = *normal;
  *body2_norm = *body1_norm;
  //mtv faces in positive direction of some axis
  //collision normals are both set to be in positive dir
  //if l1 < l2, want to want to move l1 in negative dir
  //else move l2 in negative dir
  if (l1 < l2) {
    vector_2_scale(body1_norm, -1, body1_norm);
  }
  else {
    vector_2_scale(body2_norm, -1, body2_norm);
  }
}

void solve_for_finals(double m1, double m2, double v1i, double v2i, double* v1f, double* v2f) {
  //solves for final velocities in an elastic colliison
  if (isinf(m1) && isinf(m2)) {
    *v1f = v1i;
    *v2f = v2i;    
  }
  else if (isinf(m1) || isinf(m2)) {
    double m1scale = 1;
    double m2scale = -1;
    if (isinf(m2)) {
      m1scale = -1;
      m2scale = 1;
    }
    *v1f = v1i * m1scale;
    *v2f = v2i * m2scale;    
  }
  else {
    *v2f = (m2 * v2i - m1 * (v2i - 2 * v1i)) / (m1 + m2);
    *v1f = *v2f + v2i - v1i;
  }
}

void elastic_reduce(double m1, double m2, double* f1f, double* f2f, double els) {
  //based on elasticity paras, modify velocities a bit
  //intented to be between 0-1, going outside would do weird things
  //0 represents no elasticity, 1 represents full elasticity
  double avgP = (m1 * (*f1f) + m2 * (*f2f)) / 2;
  *f1f = els * (*f1f) + (1 - els) * avgP;
  *f2f = els * (*f2f) + (1 - els) * avgP;
}


void impact(body* b1, body* b2, vector_2* normal) {
  double m1 = get_mass(b1), m2 = get_mass(b2);
    
  vector_2 *b1v = get_velocity(b1) , *b2v = get_velocity(b2);
  
  double body1i = 0, body2i = 0;
  double body1f = 0, body2f = 0;
  double body1d = 0, body2d = 0;
  double scale = 0;
  vector_2 body1add = *zero_vec, body2add = *zero_vec;
  fizzle* f1 = get_fizzle(b1), *f2 = get_fizzle(b2);
  body1i = get_projected_length_vec(b1v, normal);
  body2i = get_projected_length_vec(b2v, normal);
  

  
  solve_for_finals(m1, m2, body1i, body2i, &body1f, &body2f);
  
  body1d = body1f - body1i;
  body2d = body2f - body2i;

  scale = (get_bounce(f1) + get_bounce(f2)) / 2.0;

  body1d *= scale;
  body2d *= scale;

  vector_2_scale(normal, body1d, &body1add);
  vector_2_scale(normal, body2d, &body2add);


  add_impact(f1, &body1add);
  add_impact(f2, &body2add);
}

tether* tether_bodies(body* b1, body* b2, tether* tether_params) {
  tether* teth = create_tether_blank(get_body_center(b1), get_body_center(b2), get_fizzle(b1), get_fizzle(b2));
  copy_tether_params(tether_params, teth);
  return teth;
}

