
#include "body.h"
#include <math.h>

void impact(body* b1, body* b2, vector_2* normal);


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
      //fprintf(stderr, "detecting  a collision!\n");
      resolve_collision(map, main_body, currBody);
    }
    currNode = currNode->next;
  }
  clean_collider_list(&list);

}

void resolve_collision(spatial_hash_map* map, body* body1, body* body2) {
  polygon* p1 = body1->coll->shape;
  polygon* p2 = body2->coll->shape;
  vector_2 normal_of_collision;
  vector_2 b1_norm = *zero_vec;
  vector_2 b2_norm = *zero_vec;
  
  int actual_collision = find_mtv_of_polygons(p1, p2, &normal_of_collision);
  double mtv_mag = vector_2_magnitude(&normal_of_collision);
  //potentially theres' no actual collision since everything has been coarse grain at this point
  if (actual_collision != 0) {
    make_unit_vector(&normal_of_collision, &normal_of_collision);
    get_normals_of_collision(body1, body2, &normal_of_collision, &b1_norm, &b2_norm);
    displace_bodies(map,body1, body2, mtv_mag, &b1_norm, &b2_norm);
    //in addition to setting velocities, also need to displace object outside of eachother
    impact(body1, body2, &b1_norm);
  }
}

//returns an inverted mass contribution
//out of sum of both masses, fill m1 with mass contribution of m2, and vice versa
//used quite a bit when I want heavy objects to react less when interacting with lighter objects
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
  double b1Mass = getMass(b1);
  double b2Mass = getMass(b2);

  inv_mass_contribution(b1Mass, b2Mass, &b1Scale, &b2Scale);

  b1Scale *= mtv_mag;
  b2Scale *= mtv_mag;
  vector_2_scale(&b1tv, b1Scale, &b1tv);
  vector_2_scale(&b2tv, b2Scale, &b2tv);  

  
  vector_2_to_virt_pos(&b1tv, &b1d);
  vector_2_to_virt_pos(&b2tv, &b2d);

  
  if (b1d.x != 0) {
    b1d.x += (b1d.x > 0) ? 1: -1;
  }
  if (b1d.y != 0) {
    b1d.y += (b1d.y > 0) ? 1: -1;
  }
  
  if (b2d.x != 0) {
    b2d.x += (b2d.x > 0) ? 1: -1;
  }
  
  if (b2d.y != 0) {
    b2d.y += (b2d.y > 0) ? 1: -1;
  }
  
  update(map, b1->coll, &b1d, 0);
  update(map, b2->coll, &b2d, 0);
}


void get_normals_of_collision(body* body1, body* body2, vector_2* normal, vector_2* body1_norm, vector_2* body2_norm) {
  double l1, l2;
  l1 = get_projected_length_pos(getCenter(body1), normal);
  l2 = get_projected_length_pos(getCenter(body2), normal);
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


struct fizzle_struct* get_fizzle(body* body) { return body->fizz; }

struct collider_struct * get_collider(body* body) { return body->coll; }

body* createBody(fizzle* fizz, struct collider_struct* coll) {
  body* new = malloc(sizeof(body));
  new->fizz = fizz;
  new->coll = coll;
  new->polt = NULL;
  coll->body = new;
  new->owner = NULL;
  
  return new;
}

void free_body(body* rm) {
  free_fizzle(rm->fizz);
  free_collider(rm->coll);
  free(rm);
}


fizzle* getFizzle(body* aBody) {
  return aBody->fizz;
}

double getMass(body* aBody) {
  return aBody->fizz->mass;
}

vector_2* getVelocity(body* aBody) {
  return &(aBody->fizz->velocity);
}

virt_pos* getCenter(body* aBody) {
   return aBody->coll->shape->center;
}



//two equations, based on conservation of mass and momentum
//holds for elastic colliisons
/*
  vf1 + vi1 = vf2 + vi2
  VVV this was wrong actually VVV
  m1(vf1 - vi1) = m2(vf2 - vi2)
  messed up simplification, here is original
  m1(vi1) + m2(vi2) = m1(vf1) + m2(vf2) - >
  m1(vf1 - vi1) = -m2(vf2 - fi2)
  whoops
  vf1 + vi1 = vf2 + vi2 ->
  vf1 = vf2 + vi2 - vi1

  m1(vf2 + vi2 - vi1 - vi1) = -m2(vf2 - fi2)
  m1*vf2 + m1(vi2 - 2vi1) = -m2*vf2 + m2*fi2
  m1*vf2 + m2*vf2 = +m2*fi2 - m1(vi2 - 2vi1)
  vf2(m1 + m2) = +m2*fi2 - m1(vi2 - 2vi1)
  vf2 = (+m2*vi2 - m1(vi2 - 2vi1)) / (m1 + m2)
 */

void solveForFinals(double m1, double m2, double v1i, double v2i, double* v1f, double* v2f) {
  //solves for final velocities in an elastic colliison
  //apparently c double arithmatic leaves things to be desired for infinity
  //outline for goal behavior
  /*
    if both inf, vf = vi
    if one is inf, sdfgsdfg
    just negate velocity? idk
    if neither is inf, do maths
   */
  if (isinf(m1) && isinf(m2)) {
    //just stop
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

void elasticReduce(double m1, double m2, double* f1f, double* f2f, double els) {
  //based on elasticity paras, modify velocities a bit
  //intented to be between 0-1, going outside would do weird things
  //0 represents no elasticity, 1 represents full elasticity
  double avgP = (m1 * (*f1f) + m2 * (*f2f)) / 2;
  *f1f = els * (*f1f) + (1 - els) * avgP;
  *f2f = els * (*f2f) + (1 - els) * avgP;
}


void impact(body* b1, body* b2, vector_2* normal) {
  double m1 = getMass(b1), m2 = getMass(b2);
    
  vector_2 *b1v = getVelocity(b1) , *b2v = getVelocity(b2);
  
  double body1i = 0, body2i = 0;
  body1i = get_projected_length_vec(b1v, normal);
  body2i = get_projected_length_vec(b2v, normal);
  
  double body1f = 0, body2f = 0;
  
  solveForFinals(m1, m2, body1i, body2i, &body1f, &body2f);
  
  //double param = 1;
  //elasticReduce(m1, m2, &body1f, &body2f, param);
  
  double body1d = body1f - body1i;
  double body2d = body2f - body2i;

  vector_2 body1add = *zero_vec, body2add = *zero_vec;
  vector_2_scale(normal, body1d, &body1add);
  vector_2_scale(normal, body2d, &body2add);


  fizzle* fizz1 = getFizzle(b1);
  fizzle* fizz2 = getFizzle(b2);

  add_impact(fizz1, &body1add);
  add_impact(fizz2, &body2add);
}

