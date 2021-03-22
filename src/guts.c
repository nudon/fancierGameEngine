#include "guts.h"
#include "game_state.h"
#include "input.h"
#include "builder.h"
#include "names.h"
#include "gi_structs.h"

void resolve_collisions(spatial_hash_map* map, body* main_body) {
  collider* main_coll = get_collider(main_body);
  
  vector* occupied = main_coll->collider_node->active_cells;
  gen_list list;
  init_gen_list(&list);
  store_unique_colliders_in_list(map, occupied, &list);
  gen_node* currNode = list.start;
  body* currBody;
  while(currNode != NULL) {
    currBody =((collider*)currNode->stored)->body;
    if (foreign_body(main_body, currBody)) {
      resolve_collision(map, main_body, currBody);
    }
    currNode = currNode->next;
  }
  clean_collider_list(&list);

}

void resolve_collision(spatial_hash_map* map, body* body1, body* body2) {
  polygon* p1 = get_polygon(get_collider(body1));
  polygon* p2 = get_polygon(get_collider(body2));
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
      //draw_virt_pos(getCam(), &poc);
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
  
  b1_line = vector_between_points(&b1_c, poc);
  b2_line = vector_between_points(&b2_c, poc);

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


event* make_load_event(virt_pos* cent) {
  polygon* poly = createNormalPolygon(7);
  set_center(poly, cent);
  event* load = make_event(poly);
  return load;
}


void insert_load_zone_into_plane(char* from_map, char* to_map, plane* from_plane, char* to_plane_name, virt_pos* from_pos, virt_pos* to_pos) {
  char* from_plane_name = get_plane_name(from_plane);
  event* load_event = make_load_event(from_pos);
  load_zone* lz = make_load_zone(from_map, to_map, from_plane_name, to_plane_name, to_pos, load_event);
  add_load_zone_to_plane(from_plane, lz);
}

void make_compound_user(compound* comp) {
  body* head = get_compound_head(comp);
  poltergeist* polt = make_poltergeist();
  smarts * sm = get_compound_smarts(comp);
  flags* comp_flags = get_comp_flags(sm);
  give_user_poltergeist(polt);
  set_poltergeist(head, polt);
  set_user(comp_flags, 1);
  set_travel(comp_flags, 1);
  setUser(comp);
}

void make_compound_builder(compound* comp) {
  body* head = get_compound_head(comp);
  poltergeist* polt = make_poltergeist();
  give_builder_poltergeist(polt);
  set_poltergeist(head, polt);
  setBuilder(comp);
}


void jump_action(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = smarts_get_comp_stats(sm);
  body* b = NULL;
  fizzle* fizz = NULL;
  vector_2 jump_force = *g;
  vector_2_scale(&jump_force, 0.25, &jump_force);
  double jump_mag = 50;
  if (c_stats->jump_airtime == -1) {
    if (c_stats->jumps_left > 0) {
      c_stats->jumps_left--;
      c_stats->jump_airtime = 0;
    }
    else {
      return;
    }
  }
  jump_mag *= (1 - (c_stats->jump_airtime / c_stats->max_jump_airtime));
  //fprintf(stderr,"jumping with mag = %f, air_t = %f and max air_t = %f\n", jump_mag, c_stats->jump_airtime, c_stats->max_jump_airtime);
  //fprintf(stderr, "dt is %f\n", get_dT());
  vector_2_scale(&jump_force, -1 * jump_mag ,&jump_force);
  b = get_compound_head(c);
  fizz = get_fizzle(b);
  add_tether(fizz, &jump_force);
  /*
  for (gen_node* curr = get_bodies(c)->start; curr != NULL; curr = curr->next) {
    aBody = (body*)curr->stored;
    fizz = get_fizzle(aBody);
    add_tether(fizz, &jump_force);
  }
  */
  c_stats->jump_airtime += get_dT();
  if (c_stats->jump_airtime > c_stats->max_jump_airtime) {
    c_stats->jump_airtime = -1;
  }
}

void end_jump(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = smarts_get_comp_stats(sm);
  if (c_stats->jump_airtime != -1) {
    c_stats->jumps_left--;
    c_stats->jump_airtime = -1;
  }
}

void jump_action_reset(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = smarts_get_comp_stats(sm);
  c_stats->jumps_left = c_stats->max_jumps;
  c_stats->jump_airtime = -1;
}

void pickup_action(compound* c) {
  gen_node* curr = get_bodies(c)->start;
  gen_node* events;
  body* b;
  event* e;
  int done = 0;
  spatial_hash_map* shm = get_shm(get_plane_by_name(getMap(), MAIN_PLANE_NAME));
  while(!done && curr!=NULL) {
    b = (body*)curr->stored;
    events = get_body_events(b)->start;
    while(!done && events!=NULL) {
      e = (event*)events->stored;
      if (strcmp(get_event_name(e), "grab") == 0 && get_shared_input(b) == NULL) {
	check_event(shm, e);
	if (get_shared_input(b) != NULL) {
	  set_temp_owner(get_shared_input(b), c);
	  done = 1;
	  printf("picked up\n");
	}
      }
      events = events->next;
    }
    curr = curr->next;
  }
}

void throw_action(compound* c) {

  gen_node* curr = get_bodies(c)->start;
  gen_node* events;
  body* b;
  event* e;
  shared_input* si = NULL;
  int done = 0;
  vector_2 throw_f = *zero_vec;
  double throw_s = 6;
  fizzle* obj_f = NULL;
  while(!done && curr!=NULL) {
    b = (body*)curr->stored;
    events = get_body_events(b)->start;
    while(!done && events!=NULL) {
      e = (event*)events->stored;
      if (strcmp(get_event_name(e), "grab") == 0) {
	//so far holders are individual bodies that use other compounds si
	//removing should si will release object
	si = get_shared_input(b);
	if (si != NULL) {
	  obj_f = get_fizzle(b);
	  un_set_shared_input(b);
	  un_set_temp_owner(si);
	  get_tether(get_fizzle(b), &throw_f);
	  vector_2_scale(&throw_f, -1 * throw_s, &throw_f);
	  add_tether(obj_f, &throw_f);
	  //also need to reset holdable flags
	  compound* c = get_smarts_compound(get_body_smarts(shared_input_get_tracking_body(si)));
	  flags* c_flags = get_comp_flags(get_compound_smarts(c));
	  set_holdable(c_flags, 1);					  
	  done = 1;
	  printf("thrown\n");
	}
      }
      events = events->next;
    }
    curr = curr->next;
  }
}

void set_temp_owner(shared_input* si, compound* own) {
  compound* c = get_owner(shared_input_get_tracking_body(si));
  gen_node* curr = get_bodies(c)->start;
  body* b = NULL;
  while(curr != NULL) {
    b = (body*)curr->stored;
    set_owner(b,own);
    curr = curr->next;
  }
}


void un_set_temp_owner(shared_input* si) {
  compound* c = get_smarts_compound(get_body_smarts(shared_input_get_tracking_body(si)));
  gen_node* curr = get_bodies(c)->start;
  body* b = NULL;
  while(curr != NULL) {
    b = (body*)curr->stored;
    set_owner(b,c);
    curr = curr->next;
  }
}

vector_2 vision_inv_distance_scale(vector_2* vec) {
  double mag = vector_2_magnitude(vec);
  double eq = 100;
  double scale = 0;
  vector_2 ret = *zero_vec;
  if (mag == 0) {
    fprintf(stderr, "warning, gave zero_vec for line of sight to inv_distance_scale\n"); 
    return ret;
  }
  if (mag < eq) {
    scale = 1;
  }
  else {
    scale = eq / mag;
  }
  scale *= eq / mag;
  vector_2_scale(vec, scale, &ret);
  return ret;
}

vector_2 vision_speed_scale(vector_2* vec, vector_2* velocity) {
  double mag = vector_2_magnitude(vec);
  double vel = vector_2_magnitude(velocity);
  double eq = 50;
  double  scale;
  vector_2 ret = *zero_vec;
  if (vel == 0) {
    fprintf(stderr, "warning, gave zero_vec for line of sight to speed_scale\n"); 
    scale = 0;
    return ret;
  }
  if (vel < eq) {
    scale = vel / eq;
  }
  else {
    scale = 1;
  }
  scale *= eq / mag;
  vector_2_scale(vec, scale, &ret);
  return ret;
}
 
 
void user_poltergeist(body* user_body, vector_2* t_disp, double* r_disp) {
  get_input_for_body(user_body, t_disp, r_disp);
  reorient(user_body, x_axis, t_disp, r_disp);
}

void builder_poltergeist(body* builder, vector_2* t_disp, double* r_disp) {
  builder_input(builder, t_disp, r_disp);
}

void basic_brain(body* b, vector_2* t_disp, double* r_disp) {
  vector_2 use, danger, move;
  compound* c = get_owner(b);
  smarts* sm = get_compound_smarts(c);
  use = get_from_smarts(sm, SM_USEFULL);
  danger = get_from_smarts(sm, SM_DANGER);
  move = get_from_smarts(sm, SM_MOVE);
  double use_mag = vector_2_magnitude(&use);
  double danger_mag = vector_2_magnitude(&danger);
  if (use_mag > 100) {
    pickup_action(c);
  }
  if (danger_mag > 100) {
    throw_action(c);
  }
  translate(b, &move, t_disp, r_disp);
  reorient(b, x_axis, t_disp, r_disp);
}

void standard_poltergeist(body* body, vector_2* t_disp, double* r_disp) {
  smarts* sm = get_body_smarts(body);
  if (sm == NULL) {
    return;
  }
  vector_2 dir = get_from_smarts(sm, SM_MOVE);
  translate(body,&dir, t_disp, r_disp);
  //also reorient so body is "facing" dir
  reorient(body, &dir, t_disp, r_disp);
}

void look_poltergeist(body* body, vector_2* t_disp, double* r_disp) {
  smarts* sm = get_compound_smarts(get_owner(body));
  if (sm == NULL) {
    return;
  }
  vector_2 c_dir = get_from_smarts(sm, SM_LOOK);
  if (isCloseEnoughToZeroVec(&c_dir)) {
    return;
  }
  reorient(body, &c_dir, t_disp, r_disp);
  //printf("\n\nrotation force of %f for vector", *r_disp);
  // print_vector(&c_dir);
}

//takes body, applys a constant force in direction of torsos velocity
//paired with a tether between body and torso, holds body in front of torso
void holder_poltergeist(body* b, vector_2* t_disp, double* r_disp) {
  compound* c = get_owner(b);
  smarts* c_sm = get_compound_smarts(c);
  body* head = get_compound_head(c);
  vector_2 dir = *zero_vec, offset = *zero_vec;
  double theta;
  vector_2 temp = *zero_vec;
  fizzle* fizz = get_fizzle(b);
  fizzle* base_fizz = get_base_fizzle(b);
  offset = vector_between_bodies(head, b);
  theta = angle_of_vector(&offset);
  if (offset.v1 < 0) {
    set_reflections(get_polygon(get_collider(b)), -1,1);
    offset.v1 *= -1;
    offset.v2 *= -1;
    theta = angle_of_vector(&offset);
  }
  else {
    set_reflections(get_polygon(get_collider(b)), 1,1);
  }
  push_shared_reflections(b);
  set_rotation(get_polygon(get_collider(b)), theta);
  get_tether(base_fizz, &temp);
  set_tether(base_fizz, zero_vec);
  add_tether(fizz, &temp);
  
  dir = get_from_smarts(c_sm, SM_MOVE);
  translate(b, &dir, t_disp, r_disp);
}

event* make_basic_vision_event(body* b) {
  polygon* event_area = vision_triangle(150, 200, 0);
  event* e = make_event(event_area);
  set_event_by_name(e, "basic_decide_event");
  add_event_to_body(b,e);
  return e;  
}

//side vision, event should prioritize fast moving things, result is rotating body towards thing
event* make_side_vision_event(body* b) {
  polygon* event_area = vision_cone(150, 110, 5, 0);
  event* e = make_event(event_area);
  set_event_by_name(e, "side_sight");
  add_event_to_body(b,e);
  return e;  
}

//main vision, event should prioritize the closest thing in range, result is anaylzing thing in range and ?
event* make_main_vision_event(body* b) {
  polygon* event_area = vision_cone(220, 20, 2, 0);
  event* e = make_event(event_area);
  set_event_by_name(e, "main_sight");
  add_event_to_body(b,e);
  return e;  
}

//hearing, should behave like side vision by noticing things that are moving quickly
event* make_hearing_event(body* b) {
  polygon* event_area = createNormalPolygon(9);
  set_scale(event_area, 20);
  event* e = make_event(event_area);
  set_event_by_name(e, "side_sight");
  add_event_to_body(b,e);
  return e;  
}

event* make_foot_step_event(body* b) {
  polygon* area = clonePolygon(get_polygon(get_collider(b)));
  event* e = make_event(area);
  set_event_by_name(e, "foot_step");
  add_event_to_body(b,e);
  return e;
}

event* make_grab_event(body* b) {
  polygon* area = clonePolygon(get_polygon(get_collider(b)));
  set_scale(area, get_scale(area) + 3);
  event* e = make_event(area);
  set_event_by_name(e, "grab");
  add_event_to_body(b,e);
  set_auto_check(e, 0);
  return e;
}

void side_sight_event(event* e, body* b2, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
  smarts* s_comp_sm = get_compound_smarts(self_comp);
  vector_2 vel = *zero_vec, add = *zero_vec;
  get_velocity(get_fizzle(b2), &vel);
  if (foreign_body(self, b2) && !isCloseEnoughToZeroVec(&vel)) {
    add = vector_between_bodies(self, b2);

    add = vision_speed_scale(&add, &vel);
    add_to_smarts(s_comp_sm, SM_LOOK,  &add);
  }
}


event* make_builder_select_event(body* b) {
  polygon* event_area = createNormalPolygon(9);
  set_scale(event_area, 2);
  event* e = make_event(event_area);
  set_event_by_name(e, "builder_select");
  add_event_to_body(b,e);
  return e;  
}

void builder_select_event(event* e, body* b2, virt_pos* poc) {
  compound* select_comp = get_owner(b2);
  set_builder_selected_item(select_comp);
}

void main_sight_event(event* e, body* b2, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
  compound* trigger_comp = get_owner(b2);

  smarts* t_comp_sm = get_compound_smarts(trigger_comp);
  smarts* s_comp_sm = get_compound_smarts(self_comp);
  if (t_comp_sm == NULL) {
    return;
  }
  flags* trig_flags = get_comp_flags(t_comp_sm);
  flags* self_flags = get_comp_flags(s_comp_sm);
  vector_2 dir = *zero_vec;
  int run = is_hunter(trig_flags) && is_prey(self_flags);
  int chase = is_hunter(self_flags) && is_prey(trig_flags);
  int pickup  = is_holdable(trig_flags);
  if (foreign_body(self, b2)) {
    if (pickup) {
      dir = vector_between_bodies(self, b2);
      dir = vision_inv_distance_scale(&dir);
      add_to_smarts(s_comp_sm, SM_USEFULL, &dir);
    }
    if (chase && run) {
      dir = vector_between_bodies(self, b2);
      dir = vision_inv_distance_scale(&dir);
      add_to_smarts(s_comp_sm, SM_ATTACK, &dir);
    }
    if (chase) {
      dir = vector_between_bodies(self, b2);
      dir = vision_inv_distance_scale(&dir);
      add_to_smarts(s_comp_sm, SM_USEFULL, &dir);
    }
    if (run) {
      dir = vector_between_bodies(b2 , self);
      dir = vision_inv_distance_scale(&dir);
      add_to_smarts(s_comp_sm, SM_DANGER, &dir);
    }
  }    
}



void foot_placement(event* e, body* trigger, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
  compound* trigger_comp = get_owner(trigger);
  
  vector_2 dir = *zero_vec;
  virt_pos tc = get_body_center(trigger);
  virt_pos sc = get_body_center(self);
 
  if (self_comp != trigger_comp) {
    if (poc != NULL) {
      tc = *poc;
    }
    dir = vector_between_points(&sc, &tc);
    if (!isZeroVec(&dir)) {
      make_unit_vector(&dir, &dir);
      smarts* b_sm = get_body_smarts(self);
      add_to_smarts(b_sm, SM_MOVE, &dir);
    }
  }
}

void foot_step(event* e, body* trigger, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
    
  if (foreign_body(self, trigger)) {
    jump_action_reset(self_comp);
  }
}

void grab_event(event* e, body* trigger, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
  compound* trigger_comp = get_owner(trigger);

  smarts* t_comp_sm = get_compound_smarts(trigger_comp);
  if (t_comp_sm == NULL) {
    return;
  }
  flags* trig_flags = get_comp_flags(t_comp_sm);
 
  if (self_comp != trigger_comp) {
    if (is_holdable(trig_flags)) {
      if (get_shared_input(self) != get_shared_input(trigger)) {
	shared_input** si = get_shared_input_ref(trigger);
	virt_pos temp = shared_input_get_origin(*si);
	set_body_center(self, &temp);
	set_shared_input(self, si);
	set_holdable(trig_flags,0);
      }
    }
  }
}

