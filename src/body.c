#include <math.h>
#include "body.h"

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


shared_input* get_shared_input(body* b) {
  if (b->uniform_input != NULL) {
    return *b->uniform_input;
  }
  return NULL;
}

shared_input** get_shared_input_ref(body* b) {
  return b->uniform_input;
}

void set_shared_input(body* b, shared_input** si) {
  fizzle* f = NULL;

  f = b->fizz;
  if (b->uniform_input != NULL) {
    fprintf(stderr, "warning, overwriting a bodys shared_input");
  }
  b->uniform_input = si;
  shared_input_add_fizzle((*si), f);
  shared_input_update_rotational_offset(b);


}

void un_set_shared_input(body* b) {
  shared_input* si = get_shared_input(b);
  if (si == NULL) {
    return;
  }
  fizzle* fizz = b->fizz;
  shared_input_remove_fizzle(si, fizz);
  b->uniform_input = NULL;
}

body* createBody(fizzle* fizz, struct collider_struct* coll) {
  body* new = malloc(sizeof(body));
  new->fizz = fizz;
  new->coll = coll;
  new->polt = NULL;
  coll->body = new;
  new->owner = NULL;
  new->pic = NULL;
  new->event_list = create_gen_list();
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
  new->event_list = create_gen_list();
  new->status = 0;
  new->uniform_input = NULL;
  if (src->smarts != NULL) {
    fprintf(stderr, "Error, cloning was made with dumb bodies in mind and I did not support cloning smart bodies\n");
    new->smarts = NULL;
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
  free_body_smarts(rm);
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
  return get_end_fizzle(get_base_fizzle(b));
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


void offset_body(body* b, virt_pos* offset) {
  virt_pos cent;
  cent = get_body_center(b);
  virt_pos_add(&cent, offset, &cent);
  set_body_center(b, &cent);
}

void set_body_gravity(body* b, vector_2* grav) {
  set_gravity(get_fizzle(b), grav);
  set_gravity(get_base_fizzle(b), grav);    
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
  list_prepend(b->event_list, create_gen_node(e));
  set_event_body(e, b);
}

gen_list* get_body_events(body* b) {
  return b->event_list;
}




tether* tether_bodies(body* b1, body* b2, tether* tether_params) {
  polygon* p1 = get_polygon(get_collider(b1));
  polygon* p2 = get_polygon(get_collider(b2));
  tether* teth = create_tether_blank(read_only_polygon_center(p1), read_only_polygon_center(p2), get_fizzle(b1), get_fizzle(b2));
  copy_tether_params(tether_params, teth);
  return teth;
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


void push_shared_reflections(body* b) {
  shared_input* si = get_shared_input(b);
  if (si == NULL) {
    return;
  }
  polygon* p = get_polygon(get_collider(b));
  shared_input_set_reflections(si, get_x_reflection(p), get_y_reflection(p));
}

void pull_shared_reflections(body* b) {
  shared_input* si = get_shared_input(b);
  if (si == NULL) {
    return;
  }
  polygon* p = get_polygon(get_collider(b));
  set_reflections(p,
		  shared_input_get_reflection_x(si),
		  shared_input_get_reflection_y(si));
}


int foreign_body(body* b1, body* b2) {
  return get_owner(b1) != get_owner(b2);
}

//point from b1 to b2
vector_2 vector_between_bodies(body* b1, body* b2) {
  virt_pos c1 = get_body_center(b1);
  virt_pos c2 = get_body_center(b2);
  return vector_between_points(&c1, &c2);
}
