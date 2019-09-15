#include "body.h"
#include <math.h>

int calc_contact_point(body* b1, body* b2, vector_2* mtv, virt_pos* result);
void impact_torque(body* b1, body* b2, vector_2* b1_norm, vector_2* b2_norm, virt_pos* poc, double b1_scale, double b2_scale);

struct body_stats_struct {
  int max_body_health;
  int body_health;
  //this scales incoming damage vals
  double damage_scalar;
  //after damage scalar, take (1 - (health / max_health)) * damage limiter
  //0 < x < 1 will have affect of compound taking less damage as body gets more dmage inflicted
  // 1 < x will have compound take more damapge as body is damaged
  double damage_limiter;
};

void damage_body(body* b, double amt) {
  body_stats* s = b->b_stats;
  double health_scalar;
  double limiter;
  double compound_damage = 0;
  if (s != NULL) {
    amt *= s->damage_scalar;
    health_scalar = ((double)s->body_health / s->max_body_health);
    limiter = amt * s->damage_limiter;
    amt = amt * health_scalar + limiter * (1 - health_scalar);
    amt *= 0.5;
    
    if (s->body_health < amt) {
      compound_damage = s->body_health;
      s->body_health = 0;
      //also, potentially disable poltergeist for body
      //rather then discarding the pointer, just have an additional flag in body
    }
    else {
      compound_damage = amt;
      s->body_health -= amt;
    }
    //apply compound_damge to controlling compound
    compound* c = get_owner(b);
    damage_compound(c, compound_damage);
  }
}
  
  
enum shared_input_mode{si_add, si_avg};

struct shared_input_struct {
  virt_pos t_disp;
  int t_count;
  double r_disp;
  int r_count;
  virt_pos avg_t;
  double avg_r;
  virt_pos* shared_input_origin;
  enum shared_input_mode mode;
};

shared_input* create_shared_input() {
  shared_input* new = malloc(sizeof(shared_input));
  new->t_disp = *zero_pos;
  new->t_count = 0;
  new->r_disp = 0;
  new->r_count = 0;
  new->avg_t = *zero_pos;
  new->avg_r = 0;
  new->mode = si_add;
  new->shared_input_origin = NULL;
  return new;
}

shared_input** create_shared_input_ref() {
  shared_input** new = malloc(sizeof(char*));
  shared_input* inner = create_shared_input();
  *new = inner;
  return new;
}

void free_shared_input_ref(shared_input** rm) {
  if (rm != NULL && *rm != NULL) {
    free_shared_input(*rm);
    *rm = NULL;
  }
}

void free_shared_input(shared_input* rm) {
  free(rm);
}

shared_input* get_shared_input(body* b) {
  if (b->uniform_input != NULL) {
    return *b->uniform_input;
  }
  return NULL;
}

void set_shared_input(body* b, shared_input** si) {
  if (b->uniform_input != NULL) {
    fprintf(stderr, "warning, overwriting a bodys shared_input");
  }
  b->uniform_input = si;
}

void add_to_shared_input(virt_pos* t, double r, shared_input* si) {
  if (si->mode == si_avg) {
    si->avg_t = *zero_pos;
    si->avg_r = 0;
    si->mode = si_add;
  }
  if (!isZeroPos(t)) {
      virt_pos_add(t, &(si->t_disp), &(si->t_disp));
      si->t_count++;
  }
  if (r != 0.0) {
    si->r_disp += r;
    si->r_count++;
  }
}

void get_avg_movement(shared_input* si, virt_pos* t, double* r) {
  if (si->mode == si_add) {
    if (si->t_count != 0) {
      si->avg_t.x = si->t_disp.x / si->t_count;
      si->avg_t.y = si->t_disp.y / si->t_count;
    }
    if (si->r_count != 0) {
      si->avg_r = si->r_disp / si->r_count;
    }
    si->t_disp = *zero_pos;
    si->t_count = 0;
    si->r_disp = 0;
    si->r_count = 0;
    si->mode = si_avg;
  }
  if (t != NULL) {
    *t = si->avg_t;
  }
  if (r != NULL) {
    *r = si->avg_r;
  }
}

void set_shared_input_origin(shared_input* si, virt_pos* point) {
  if (si->shared_input_origin != NULL) {
    fprintf(stderr, "warning, overwriting shared input origin\n");
  }
  si->shared_input_origin = point;
}

virt_pos get_shared_input_origin(shared_input* si) {
  virt_pos ret = *zero_pos;
  if (si != NULL) {
    if (si->shared_input_origin != NULL) {
      ret = *si->shared_input_origin;
    }
    else {
      fprintf(stderr, "warning, shared_input_origin not set\n");
    }
  }
  return ret;
}

void clear_shared_input(shared_input* si) {
  si->mode = si_add;
  si->t_disp = *zero_pos;
  si->t_count = 0;
  si->r_disp = 0;
  si->r_count = 0;
}

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
  new->uniform_input = NULL;
  new->b_stats = NULL;
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
  new->uniform_input = NULL;
  if (src->b_stats != NULL) {
    *new->b_stats = *src->b_stats;
  }
  else {
    new->b_stats = NULL;
  }
  return new;
}

void free_body(body* rm) {
  free_fizzle(rm->fizz);
  free_collider(rm->coll);
  free_shared_input_ref(rm->uniform_input);
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

virt_pos get_body_center(body* b) {
  return get_center(get_polygon(get_collider(b)));
}

void set_body_center(body* b, virt_pos* vp) {
  set_center(get_polygon(get_collider(b)), vp);
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
    double b1d, b2d;
    virt_pos poc;
    impact(body1, body2, &b1_norm, &b1d, &b2d);

    if (calc_contact_point(body1, body2, &b1_norm, &poc) > 0) {
      draw_virt_pos(getCam(), &poc);
      impact_torque(body1, body2, &b1_norm, &b2_norm, &poc, b1d, b2d);
      }
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
  double b1Mass = get_mass(get_fizzle(b1));
  double b2Mass = get_mass(get_fizzle(b2));

  inv_mass_contribution(b1Mass, b2Mass, &b1Scale, &b2Scale);

  b1Scale *= mtv_mag;
  b2Scale *= mtv_mag;
  vector_2_scale(&b1tv, b1Scale, &b1tv);
  vector_2_scale(&b2tv, b2Scale, &b2tv);  

  
  vector_2_to_virt_pos(&b1tv, &b1d);
  vector_2_to_virt_pos(&b2tv, &b2d);

  move_body(map, b1, &b1d, 0.0);
  move_body(map, b2, &b2d, 0.0);
  
}


void get_normals_of_collision(body* body1, body* body2, vector_2* normal, vector_2* body1_norm, vector_2* body2_norm) {
  double l1, l2;
  virt_pos b1c = get_body_center(body1);
  virt_pos b2c = get_body_center(body2);
  l1 = get_projected_length_pos(&b1c, normal);
  l2 = get_projected_length_pos(&b2c, normal);
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


void impact(body* b1, body* b2, vector_2* normal, double* b1d, double* b2d) {
  fizzle* f1 = get_fizzle(b1);
  fizzle* f2 = get_fizzle(b2);
  double m1 = get_mass(f1), m2 = get_mass(f2);
    
  vector_2 b1v = *zero_vec, b2v = *zero_vec;
  get_velocity(f1, &b1v);
  get_velocity(f2, &b2v);
  
  double body1i = 0, body2i = 0;
  double body1f = 0, body2f = 0;
  double body1d = 0, body2d = 0;
  double scale = 0;
  vector_2 body1add = *zero_vec, body2add = *zero_vec;
  body1i = get_projected_length_vec(&b1v, normal);
  body2i = get_projected_length_vec(&b2v, normal);
  

  
  solve_for_finals(m1, m2, body1i, body2i, &body1f, &body2f);
  
  body1d = body1f - body1i;
  body2d = body2f - body2i;

  if (b1d != NULL) {
    *b1d = body1d;
  }
  if (b2d != NULL) {
    *b2d = body2d;
  }

  scale = (get_bounce(f1) + get_bounce(f2)) / 2.0;

  body1d *= scale;
  body2d *= scale;

  vector_2_scale(normal, body1d, &body1add);
  vector_2_scale(normal, body2d, &body2add);


  add_impact(f1, &body1add);
  add_impact(f2, &body2add);
}

//some ideas for getting point of impact
/*
  have mtv
  keep track of which points are the min/max projections
  should also be able to see points who are extreme because they are the base of normals
  should be one point which is the embedded point, use that after application of mtv for point of contact

  solution

  have a super extreme projections
  collect min/max projection length and points
  then also collect the second closest points and use those 
  using primary projection point differences determine which primary points are candidates
  then take secondary points, and their secondsary lengths, 
  if both are small(near zero, indicating they are on corners of side with mtv as norm) then take distance between primary and secondary
  smallest distance pair uses midpoint as point of contact
  if pair of lengths is small and another is large, use the large distance primary point as the point of conta
 */

int calc_contact_point(body* b1, body* b2, vector_2* mtv, virt_pos* result) {
  polygon* p1 = get_polygon(get_collider(b1));
  polygon* p2 = get_polygon(get_collider(b2));

  virt_pos p1_min, p1_max, p1_sec_min, p1_sec_max, p2_min, p2_max, p2_sec_min, p2_sec_max;
  virt_pos p1_e, p1_e_sec, p2_e, p2_e_sec;
  double d1, d2;

  double thresh = 1.5;
  virt_pos contact_point = *zero_pos;
  int ret = 0;
  
  tmi_points_of_polygon(p1, mtv, &p1_min, &p1_max, &p1_sec_min, &p1_sec_max);
  tmi_points_of_polygon(p2, mtv, &p2_min, &p2_max, &p2_sec_min, &p2_sec_max);
  
  d1 = fabs(get_projected_length(&p1_min, mtv) - get_projected_length(&p2_max, mtv));
  d2 = fabs(get_projected_length(&p2_min, mtv) - get_projected_length(&p1_max, mtv));

  if (d1 < d2) {
    p1_e = p1_min;
    p1_e_sec = p1_sec_min;
    p2_e = p2_max;
    p2_e_sec = p2_sec_max;
  }
  else {
    p1_e = p1_max;
    p1_e_sec = p1_sec_max;
    p2_e = p2_min;
    p2_e_sec = p2_sec_min;    
  }

  d1 = fabs(get_projected_length(&p1_e, mtv) - get_projected_length(&p1_e_sec, mtv));
  d2 = fabs(get_projected_length(&p2_e, mtv) - get_projected_length(&p2_e_sec, mtv));
  
  virt_pos sel_prim_p, sel_sec_p, not_prim_p, not_sec_p;
  if (d1 < thresh && d2 < thresh) {
    d1 = distance_between_points(&p1_e, &p1_e_sec);
    d2 = distance_between_points(&p2_e, &p2_e_sec);
    if (d1 < d2) {
      virt_pos_midpoint(&p1_e, &p1_e_sec, &contact_point);
      sel_prim_p = p1_e;
      sel_sec_p = p1_e_sec;
      not_prim_p = p2_e;
      not_sec_p = p2_e_sec;
    }
    else {
      virt_pos_midpoint(&p2_e, &p2_e_sec, &contact_point);
      sel_prim_p = p2_e;
      sel_sec_p = p2_e_sec;
      not_prim_p = p1_e;
      not_sec_p = p1_e_sec;
    }
    ret = 1;
  }
  else if (d1 < thresh || d2 < thresh) {
    if (d1 > d2) {
      contact_point = p1_e;
      sel_prim_p = p1_e;
      sel_sec_p = p1_e_sec;
      not_prim_p = p2_e;
      not_sec_p = p2_e_sec;
    }
    else {
      contact_point = p2_e;
      sel_prim_p = p2_e;
      sel_sec_p = p2_e_sec;
      not_prim_p = p1_e;
      not_sec_p = p1_e_sec;
    }
    ret = 2;
  }
  else {
    //unexpected
    //expecting at least one of the distances to be zero
    //because I expect mtv to be a normal of a side of a polygon
    //and the e and e_sec would be the matching endpoints of side with normal = mtv; 
    printf("this wasn't actually a good solution\n");
    printf("d1 is %f, d2 is %f\n", d1, d2);
    //exit(1);
    ret = -1;
  }

  int draw_sel = 0;
  int draw_not = 0;
  int print = 0;
  if (draw_sel) {
    draw_virt_pos(getCam(), &sel_prim_p);
    draw_virt_pos(getCam(), &sel_sec_p);
    if (print) {
      printf("\n selected \n");
      print_point(&sel_prim_p);
      print_point(&sel_sec_p);
    }
  }
  if (draw_not) {
    draw_virt_pos(getCam(), &not_prim_p);
    draw_virt_pos(getCam(), &not_sec_p);
    if (print) {
      printf("\n notselec \n");
      print_point(&sel_prim_p);
      print_point(&sel_sec_p);
    }
  }
  *result = contact_point;
  return ret;
}

void impact_torque(body* b1, body* b2, vector_2* b1_norm, vector_2* b2_norm, virt_pos* poc, double b1_scale, double b2_scale) {
  vector_2 b1_line, b2_line;
  vector_2 b1_line_p, b1_line_o, b2_line_p, b2_line_o;
  virt_pos b1_c, b2_c;
  b1_c = get_center(get_polygon(get_collider(b1)));
  b2_c = get_center(get_polygon(get_collider(b2)));
  
  vector_between_points(&b1_c, poc, &b1_line);
  vector_between_points(&b2_c, poc, &b2_line);

  decompose_vector(&b1_line, b1_norm, &b1_line_p, &b1_line_o);
  decompose_vector(&b2_line, b2_norm, &b2_line_p, &b2_line_o);

  //then, somehow scale norm orthogonols by some mass/velocity contribution
  //then add some rotational force to bodies
  fizzle* f1 = get_fizzle(b1);
  fizzle* f2 = get_fizzle(b2);
  double b1Scale = 1;
  double b2Scale = 1;
  double b1Mass = get_mass(f1);
  double b2Mass = get_mass(f2);

  inv_mass_contribution(b1Mass, b2Mass, &b1_scale, &b2_scale);
  
  double rot_scale = 0.1;
  double b1_rot = rot_scale * b1_scale * vector_2_magnitude(&b1_line_o);
  double b2_rot = rot_scale * b2_scale * vector_2_magnitude(&b2_line_o);
  
  //take b1_line, rotate 90 || M_PI / 4 u, get projected length of b1_norm_o, take sign and use as scale
  vector_2 b1_det, b2_det;
  vector_2_rotate(&b1_line_p, M_PI / 4, &b1_det);
  vector_2_rotate(&b2_line_p, M_PI / 4, &b2_det);
  //fprintf(stderr, "rot1 is %f rot2 is %f \n", b1_rot, b2_rot);
  if (get_projected_length_vec(&b1_line_o, &b1_det) < 0) {
    b1_rot *= -1;
  }

  
  if (get_projected_length_vec(&b2_line_o, &b2_det) < 0) {
    b2_rot *= -1;
  }
  add_rot_impact(get_fizzle(b1), b1_rot);
  add_rot_impact(get_fizzle(b2), b2_rot);
}

tether* tether_bodies(body* b1, body* b2, tether* tether_params) {
  polygon* p1 = get_polygon(get_collider(b1));
  polygon* p2 = get_polygon(get_collider(b2));
  tether* teth = create_tether_blank(read_only_polygon_center(p1), read_only_polygon_center(p2), get_fizzle(b1), get_fizzle(b2));
  copy_tether_params(tether_params, teth);
  return teth;
}

virt_pos get_rotational_offset(body* b) {
  virt_pos head_center = *zero_pos;
  virt_pos curr_center = *zero_pos;
  virt_pos offset = *zero_pos;
  shared_input *si = get_shared_input(b);  
  if (si != NULL) {
    head_center = get_shared_input_origin(si);
    curr_center = get_body_center(b);
    virt_pos_sub(&head_center, &curr_center, &offset);
  }
  return offset;
}

void run_body_poltergeist(body* b) {
  vector_2 t_input = *zero_vec;
  double r_input = 0;
  fizzle* f = get_fizzle(b);
  body_stats* s = b->b_stats;
  if (s != NULL && s->body_health == 0) {
    return;
  }
  apply_poltergeist(b->polt, b, &t_input, &r_input);
  add_velocity(f, &t_input);
  add_rotational_velocity(f, r_input);
}

typedef struct hitbox_struct hitbox;
typedef struct hitbox_collection_struct hitbox_collection;


struct hitbox_collection_struct {
  int size;
  int cur_index;
  collider* hitbox_array;
};

struct hitbox_struct {
  compound* hitbox_compound;
  double damage;
  double up_time;
  gen_node* node;
};

hitbox* make_hitbox(compound* c, double dmg, double tu) {
  hitbox* hb = malloc(sizeof(hitbox));
  hb->hitbox_compound = c;
  hb->damage = dmg;
  hb->up_time = tu;
  hb->node = createGen_node(hb);
  return hb;
}

//set hitbox dir by setting fizzles velocity
//more complicated projectiles can have a poltergeist(for tracking things



hitbox_collection* make_hitbox_collection(hitbox* hb, int amount) {
  hitbox_collection* hbc = malloc(sizeof(hitbox_collection));

  return hbc;
}

  
