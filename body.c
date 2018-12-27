
#include "body.h"


void resolve_collisions(spatial_hash_map* map, body* main_body) {
  collider* main_coll = main_body->coll;
  vector* occupied = main_coll->collider_node->active_cells;
  int size = number_of_unique_colliders_in_entries(map, occupied);
  body* curr;
  collider* collisions[size];
  unique_colliders_in_entries(map, occupied, collisions);
  for (int i = 0; i < size; i++) {
    curr = collisions[i]->body;
    if (curr != main_body) {
      resolve_collision(map, main_body, collisions[i]->body);
    }
  }
}

void resolve_collision(spatial_hash_map* map, body* body1, body* body2) {
  //goal, add some forces to either body to simulate a collision

    //cases this needs to cover
  //case of a large object hitting a small one, should cause the small one to move with it, with little change in own direction
  //opposite case, small object should rebound off of large object somehonew
  //need to find some general principle that determines how a vector can rebound like that
 
  //in my elucidite wisdom my notes on impulse are a 6th of a page,
  //given two object, depening on things(mass or momentum?), can determine resulting directions of motion
  //for object, colliding with another object on its  side with normal vector, decompose vector into parallell and orthogonal components
  //beleive orhogonal component stays the same, parallell component is variable though.
  //thnk it gets scaled by (mass1 - mass2), or some such. if mass1 >>>> mass2, things hardly change. if equal, cancles motion for obj 1, if mass1 <<<< mass2, object1 rebounds reflecting off of normal

  //messed up a bit
  //if mass of objects are equal
  //then both objects should move with same velocity in end. depending on how many objects are moving
  //final velocity may be zero or anything else really.
  //also think I need to displace objects by mtv along with changing velocities
 

  
  polygon* p1 = body1->coll->shape;
  polygon* p2 = body2->coll->shape;
  vector_2 normal_of_collision;
  vector_2 b1_norm = *zero_vec;
  vector_2 b2_norm = *zero_vec;
  
  int actual_collision = find_mtv_of_polygons(p1, p2, &normal_of_collision);
  double mtv_mag = vector_2_magnitude(&normal_of_collision);
  //potentially theres' no actual collision since everything has been coarse grain at this point
  if (actual_collision != 0) {
    //in addition to setting velocities, also need to displace object outside of eachother
    make_unit_vector(&normal_of_collision, &normal_of_collision);
    get_normals_of_collision(body1, body2, &normal_of_collision, &b1_norm, &b2_norm);
    displace_bodies(map,body1, body2, mtv_mag, &b1_norm, &b2_norm);
    impact_bodies(body1, body2, &b1_norm, &b2_norm);
    /*
    fprintf(stderr, "\n COLLISION \n");
    fprintf(stderr, "body 1 normal: ");
    print_vector(&b1_norm);
    fprintf(stderr, ", body 2 normal: ");
    print_vector(&b2_norm);
    fprintf(stderr, "\nbody 1 impact: ");
    print_vector(&(body1->fizz->impact));
    fprintf(stderr, ", body 2 impact: ");
    print_vector(&(body2->fizz->impact));
    */
  }
}

void displace_bodies(spatial_hash_map* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit) {

  vector_2 b1tv = *b1tv_unit;
  vector_2 b2tv = *b2tv_unit;
  
  virt_pos b1d = *zero_pos;
  virt_pos b2d = *zero_pos;
  //potentially weight these based off of mass like in impact bodies
  double b1Scale = 1;
  double b2Scale = 1;
  double b1Mass = getMass(b1);
  double b2Mass = getMass(b2);
  b1Scale = b2Mass / (b2Mass + b1Mass);
  b2Scale = b1Mass / (b2Mass + b1Mass);

  b1Scale *= mtv_mag;
  b2Scale *= mtv_mag;
  vector_2_scale(&b1tv, b1Scale, &b1tv);
  vector_2_scale(&b2tv, b2Scale, &b2tv);  

  
  vector_2_to_virt_pos(&b1tv, &b1d);
  vector_2_to_virt_pos(&b2tv, &b2d);

  //with these, wouldn't need the turnaryt statements
  //however non-infinite masses would always be moved, which is kind of annoying 
  //vector_2_to_virt_pos_ceil(&b1tv, &b1d);
  //vector_2_to_virt_pos_ceil(&b2tv, &b2d);
  
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

void impact_bodies(body* body1, body* body2, vector_2* b1tv_norm, vector_2* b2tv_norm) {
  //here we go!
  impact(body1, body2, b1tv_norm);
}



void get_collision_normals(body* b1, body* b2, vector_2* norm, vector_2* b1Disp, vector_2* b2Disp) {
  polygon* p1 = b1->coll->shape;
  polygon* p2 = b2->coll->shape;
  double p1Cent = get_projected_length(&(p1->center),norm);
  double p2Cent = get_projected_length(&(p2->center),norm);
  
  vector_2 p1tv = *norm;
  vector_2 p2tv = p1tv;
  
  if (p1Cent < p2Cent) {
    //going from p1 to p2 requires a positive scalar of norm
    //so p1 should move in negative dir along norm
    vector_2_scale(&p1tv, -1, &p1tv);
  }
  else {
    //reverse case of above, p2 to p1 requires a positive scalar of norm
    //so displace by negative amount
    vector_2_scale(&p2tv, -1, &p2tv);
  }
  *b1Disp = p1tv;
  *b2Disp = p2tv;
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


body* createBody(fizzle* fizz, struct collider_struct* coll) {
  body* new = malloc(sizeof(body));
  new->fizz = fizz;
  new->coll = coll;
  new->polt = NULL;
  coll->body = new;
  
  return new;
}

void freeBody(body* rm) {
  freeFizzle(rm->fizz);
  freeCollider(rm->coll);
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
   return &(aBody->coll->shape->center);
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
  *v2f = (m2 * v2i - m1 * (v2i - 2 * v1i)) / (m1 + m2);
  *v1f = *v2f + v2i - v1i;
  //*v1f = (m1 * (v2i - 2 * v1i) + m2 * v2i) / (m2 + m1);
  //*v2f = *v1f + v2i - v1i;
}

void elasticReduce(double m1, double m2, double* f1f, double* f2f, double els) {
  //based on elasticity paras, modify velocities a bit
  //officially sanctioned are else = 0,1. outside or between interval might lead to weirdness
  //0 represents no elasticity, 1 represents full elasticity
  double avgP = (m1 * (*f1f) + m2 * (*f2f)) / 2;
  *f1f = els * (*f1f) + (1 - els) * avgP;
  *f2f = els * (*f2f) + (1 - els) * avgP;
}


void impact(body* b1, body* b2, vector_2* normal) {
  //mass
  double m1 = getMass(b1), m2 = getMass(b2);
  //store velocities in here
  vector_2 *b1v = getVelocity(b1) , *b2v = getVelocity(b2);
  
  //store normal in here, normalize it
  //make_unit_vector(normal, normal);
  //store normal-parallell components of original vectors in here
  double body1i = 0, body2i = 0;
  body1i = get_projected_length_vec(b1v, normal);
  body2i = get_projected_length_vec(b2v, normal);
  
  //store final velocity components in here
  double body1f = 0, body2f = 0;
  //some elasticity param, set to 1
  double param = 1;

  //solve for final velocities
  solveForFinals(m1, m2, body1i, body2i, &body1f, &body2f);

  elasticReduce(m1, m2, &body1f, &body2f, param);
  
  //multiply unit vector normal by final velocity magnitudes
  //have final velocity components
  //for adding back into object, can either find orth comps
  //and just add the orth comp with the calculated component
  //or do some weird stuff with finding the difference between final and initial velocities
  //and adding that difference somewhere

  //doing the weird difference thing
  double body1d = body1f - body1i;
  double body2d = body2f - body2i;
  vector_2 body1add = *zero_vec, body2add = *zero_vec;
  vector_2_scale(normal, body1d, &body1add);
  vector_2_scale(normal, body2d, &body2add);


  fizzle* fizz1 = getFizzle(b1);
  fizzle* fizz2 = getFizzle(b2);

  add_impact(fizz1, &body1add);
  add_impact(fizz2, &body2add);
  
  //then add to fizzle impacs, pray and ???
}

