#include <stdio.h>
#include "shared_input.h"

enum shared_input_mode{si_add, si_avg};

struct shared_input_struct {
  virt_pos t_disp;
  int t_count;
  double r_disp;
  int r_count;
  virt_pos avg_t;
  double avg_r;
  body* tracking_body;
  polygon* tracking_poly;
  int point;
  fizzle* shared_fizzle;
  int x_reflection;
  int y_reflection;
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
  new->tracking_body = NULL;
  new->tracking_poly = NULL;
  new->point = -2;
  new->shared_fizzle = NULL;
  new->x_reflection = 1;
  new->y_reflection = 1;
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

//special logic for sharing physics along with input
//just updates total mass of shared input, and matches velocity between bodies.
void shared_input_add_fizzle(shared_input* si, fizzle* f) {
  fizzle* si_f = NULL;
  vector_2 v, si_v;
  double base_scale, si_scale;
  if (si->shared_fizzle == NULL) {
    si_f = cloneFizzle(f);
    si->shared_fizzle = si_f;
  }
  else {
    si_f = si->shared_fizzle;
    get_velocity(si_f, &si_v);
    get_velocity(f, &v);
    mass_contribution(get_mass(si_f), get_mass(f), &si_scale, &base_scale);
    vector_2_scale(&si_v, si_scale, &si_v);
    vector_2_scale(&v, base_scale, &v);
    vector_2_add(&v, &si_v, &si_v);
    set_velocity(si_f, &si_v);
    set_mass(si_f, get_mass(si_f) + get_mass(f));
  }
  redirect_fizzle(f, si_f);
}

//reverse actions of shared_input_add_fizzle
void shared_input_remove_fizzle(shared_input* si, fizzle* f) {
  fizzle* si_f = si->shared_fizzle;
  vector_2 v;
  if (get_end_fizzle(f) != si_f) {
    fprintf(stderr, "Error, trying to remove a fizzle from a shared input that was never tied to it");
    return;
  }
  set_mass(si_f, get_mass(si_f) - get_mass(f));
  get_velocity(si_f, &v);
  set_velocity(f, &v);
  clear_other_fizzle(f);
}


void shared_input_update_rotational_offset(body* b) {
  virt_pos offset = *zero_pos;
  offset = shared_input_get_rotational_offset(b);
  //print_point(&offset);
  set_rotation_offset(get_polygon(get_collider((b))), &offset);
}

//calculates rotation offset from b to si origin when b is not rotated
virt_pos shared_input_get_rotational_offset(body* b) {
  virt_pos offset = shared_input_get_offset(b);
  shared_input* si = get_shared_input(b);
  double tracker_rotation = get_rotation(si->tracking_poly);
  virt_pos_rotate(&offset, -tracker_rotation, &offset);
  return offset;
}

virt_pos shared_input_get_offset(body* b) {
  virt_pos head_center = *zero_pos;
  virt_pos curr_center = *zero_pos;
  virt_pos offset = *zero_pos;
  shared_input *si = get_shared_input(b);
  if (si != NULL) {
    head_center = shared_input_get_origin(si);
    curr_center = get_body_center(b);
    virt_pos_sub(&head_center, &curr_center, &offset);
  }
  return offset;
}

void shared_input_add_movement( shared_input* si, virt_pos* t, double r) {
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


void shared_input_update_avg_movement(shared_input* si) {
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

void shared_input_get_avg_movement(shared_input* si, virt_pos* t, double* r) {
  if (si->mode == si_add) {
    shared_input_update_avg_movement(si);
    si->mode = si_avg;
  }
  if (t != NULL) {
    *t = si->avg_t;
  }
  if (r != NULL) {
    *r = si->avg_r;
  }
}


void shared_input_set_origin(shared_input* si, body* b, int point) {
  polygon* p = get_polygon(get_collider(b));
  if (si->tracking_body != NULL) {
    fprintf(stderr, "warning, overwriting shared input origin\n");
  }
  if (point != SI_CENTER && (point < 0 || point >= get_sides(p))) {
      fprintf(stderr, "error, invalid point value for shared input, val is %d, must be between [0, %d] or %d\n", point, get_sides(p) - 1, SI_CENTER);
  }
  si->tracking_body = b;
  si->tracking_poly = p;
  si->point = point;
  
}

virt_pos shared_input_get_origin(shared_input* si) {
  virt_pos ret = *zero_pos;
  if (si != NULL) {
    if (si->tracking_poly != NULL) {
      if (si->point == SI_CENTER) {
	ret = get_center(si->tracking_poly);
      }
      else {
	get_actual_point(si->tracking_poly, si->point, &ret);
      }
    }
    else {
      fprintf(stderr, "warning, shared_input_origin not set\n");
    }
  }
  return ret;
}

body* shared_input_get_tracking_body(shared_input* si) {
  return si->tracking_body;
}

void shared_input_clear(shared_input* si) {
  si->mode = si_add;
  si->t_disp = *zero_pos;
  si->t_count = 0;
  si->r_disp = 0;
  si->r_count = 0;
}

int shared_input_get_reflection_x(shared_input* si) {
  return si->x_reflection;
}

int shared_input_get_reflection_y(shared_input* si) {
  return si->y_reflection;
}

void shared_input_set_reflections(shared_input* si, int x_r, int y_r) {
  si->x_reflection = x_r;
  si->y_reflection = y_r;
}
