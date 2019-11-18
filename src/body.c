#include "body.h"
#include <math.h>

void impact_torque(body* b1, body* b2, vector_2* b1_norm, vector_2* b2_norm, virt_pos* poc);

struct body_struct {
  fizzle* fizz;
  collider* coll;
  poltergeist* polt;
  compound* owner;
  picture* pic;
  int status;
  shared_input** uniform_input;
  //used for holding ai related events
  gen_list* event_list;
  smarts* smarts;
};

 
enum shared_input_mode{si_add, si_avg};

struct shared_input_struct {
  virt_pos t_disp;
  int t_count;
  double r_disp;
  int r_count;
  virt_pos avg_t;
  double avg_r;
  virt_pos* shared_input_origin;
  fizzle* shared_fizzle;
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
  new->shared_fizzle = NULL;
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
  fizzle* si_f = NULL, *f = NULL;
  vector_2 v, si_v;
  double base_scale, si_scale;
  f = b->fizz;
  if (b->uniform_input != NULL) {
    fprintf(stderr, "warning, overwriting a bodys shared_input");
  }
  if ((*si)->shared_fizzle == NULL) {
    si_f = cloneFizzle(f);
    (*si)->shared_fizzle = si_f;
  }
  else {
    si_f = (*si)->shared_fizzle;
    get_velocity(si_f, &si_v);
    get_velocity(f, &v);
    mass_contribution(get_mass(si_f), get_mass(f), &si_scale, &base_scale);
    vector_2_scale(&si_v, si_scale, &si_v);
    vector_2_scale(&v, base_scale, &v);
    vector_2_add(&v, &si_v, &si_v);
    set_velocity(si_f, &si_v);
    set_mass(si_f, get_mass(si_f) + get_mass(f));
    //might not be a good idea
    set_mass(si_f, get_moi(si_f) + get_moi(f));
  }
  b->uniform_input = si;
  /*
  free_fizzle(f);
  b->fizz = si_f;
  */
}

void un_set_shared_input(body* b) {
  shared_input* si = get_shared_input(b);
  if (si == NULL) {
    return;
  }
  fizzle* fizz = b->fizz;
  fizzle* si_fizz = si->shared_fizzle;
  vector_2 v;
  //undo whatever set_shared_input does
  set_mass(si_fizz, get_mass(si_fizz) - get_mass(fizz));
  set_mass(si_fizz, get_moi(si_fizz) - get_moi(fizz));
  get_velocity(si_fizz, &v);
  set_velocity(fizz, &v);
  b->uniform_input = NULL;
}

void add_to_shared_input(virt_pos* t, double r, shared_input* si) {
  if (si->mode == si_avg) {
    si->t_disp = *zero_pos;
    si->t_count = 0;
    si->r_disp = 0;
    si->r_count = 0;
    si->mode = si_add;
  }
  if (1 || !isZeroPos(t)) {
    virt_pos_add(t, &(si->t_disp), &(si->t_disp));
    si->t_count++;
  }
  if (r != r) {
    si->r_disp += r;
    si->r_count++;
  }
}


void calc_si_avg(shared_input* si) {
  if (si->t_count != 0) {
    si->avg_t.x = si->t_disp.x / si->t_count;
    si->avg_t.y = si->t_disp.y / si->t_count;
  }
  else {
    si->avg_t = *zero_pos;
  }
  if (si->r_count != 0) {
    si->avg_r = si->r_disp / si->r_count;
  }
  else {
    si->avg_r = 0;
  }
}

void get_avg_movement(shared_input* si, virt_pos* t, double* r) {
  if (si->mode == si_add) {
    calc_si_avg(si);
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
  new->smarts = NULL;
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
  //new->status = 0;
  new->uniform_input = NULL;
  if (src->smarts != NULL) {
    //allocate b_stats first 
    //*new->b_stats = *src->b_stats;
  }
  else {
    new->smarts = NULL;
  }
  return new;
}

void free_body(body* rm) {
  free_fizzle(rm->fizz);
  free_collider(rm->coll);
  free_shared_input_ref(rm->uniform_input);
  free(rm);
}

int get_move_status(body* b) {
  return b->status;
}

void set_move_status(body* b, int val) {
  b->status = val;
}

collider * get_collider(body* body) {
  return body->coll;
}

compound* get_owner(body* body) {
  return body->owner;
}

void set_owner(body* b, compound* o) {
  b->owner = o;
}

fizzle* get_fizzle(body* b) {
  shared_input* si = get_shared_input(b);
  if (si != NULL) {
    return si->shared_fizzle;
  }
  else {
    return b->fizz;
  }
}

fizzle* get_base_fizzle(body* b) {
  return b->fizz;
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
  if (actual_collision) {
    contact_damage(body1, body2);
    make_unit_vector(&normal_of_collision, &normal_of_collision);
    get_normals_of_collision(p1, p2, &normal_of_collision, &b1_norm, &b2_norm);
    
    displace_bodies(map,body1, body2, mtv_mag, &b1_norm, &b2_norm);
    virt_pos poc;
    impact(body1, body2, &b1_norm);

    if (calc_contact_point(p1, p2, &b1_norm, &poc) > 0) {
      draw_virt_pos(getCam(), &poc);
      impact_torque(body1, body2, &b1_norm, &b2_norm, &poc);
    }
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

  scale = (get_bounce(f1) + get_bounce(f2)) / 2.0;

  body1d *= scale;
  body2d *= scale;

  vector_2_scale(normal, body1d, &body1add);
  vector_2_scale(normal, body2d, &body2add);


  add_impact(f1, &body1add);
  add_impact(f2, &body2add);
}

void impact_torque(body* b1, body* b2, vector_2* b1_norm, vector_2* b2_norm, virt_pos* poc) {
  vector_2 b1_line, b2_line;
  vector_2 b1_line_p, b1_line_o, b2_line_p, b2_line_o;
  virt_pos b1_c, b2_c;
  b1_c = get_center(get_polygon(get_collider(b1)));
  b2_c = get_center(get_polygon(get_collider(b2)));
  
  vector_between_points(&b1_c, poc, &b1_line);
  vector_between_points(&b2_c, poc, &b2_line);

  decompose_vector(&b1_line, b1_norm, &b1_line_p, &b1_line_o);
  decompose_vector(&b2_line, b2_norm, &b2_line_p, &b2_line_o);

  fizzle* f1 = get_fizzle(b1);
  fizzle* f2 = get_fizzle(b2);
  
  vector_2 f1_vel, f2_vel;
  vector_2 f1_vel_p, f1_vel_o, f2_vel_p, f2_vel_o, diff;
  get_velocity(f1, &f1_vel);
  get_velocity(f2, &f2_vel);
  decompose_vector(&f1_vel, b1_norm, &f1_vel_p, &f1_vel_o);
  decompose_vector(&f2_vel, b1_norm, &f2_vel_p, &f2_vel_o);
  
  vector_2_sub(&f1_vel_p, &f2_vel_p, &diff);

  double vel_diff_scale = vector_2_magnitude(&diff);
  double b1_scale, b2_scale;
  inv_mass_contribution(get_moi(f1), get_moi(f2), &b1_scale, &b2_scale);
  
  double rot_scale = 0.04 * vel_diff_scale;
  double b1_rot = rot_scale * b1_scale * vector_2_magnitude(&b1_line_o);
  double b2_rot = rot_scale * b2_scale * vector_2_magnitude(&b2_line_o);
  
  vector_2 b1_det, b2_det;
  vector_2_rotate(&b1_line_p, M_PI / 4, &b1_det);
  vector_2_rotate(&b2_line_p, M_PI / 4, &b2_det);
  
  if (get_projected_length_vec(&b1_line_o, &b1_det) < 0) {
    b1_rot *= -1;
  }

  if (get_projected_length_vec(&b2_line_o, &b2_det) < 0) {
    b2_rot *= -1;
  }
  add_rot_impact(get_fizzle(b1), b1_rot);
  add_rot_impact(get_fizzle(b2), b2_rot);
}

//so far just calcs value/magnitude of translation
//args are radius and rotation vals
void rolling_translation(double r1, double r2, double rot1, double rot2, double* mag1, double* mag2) {
  *mag1 = -2 * r1 * rot1;
  *mag2 = -2 * r2 * rot2;
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
  apply_poltergeist(b->polt, b, &t_input, &r_input);
  add_velocity(f, &t_input);
  add_rotational_velocity(f, r_input);
}

void set_body_smarts(body* b, smarts* sm) {
  b->smarts = sm;
}

smarts* get_body_smarts(body* b) {
  return b->smarts;
}
