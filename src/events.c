#include <stdio.h>
#include "events.h"
#include "util.h"

//so, events

//basically, lots of similarity with regular collision detection
//only difference is instead of doing collision, you call some arbitrary function


//might also have some bitmask or validation function
//onTrigger might have a different method signature latter. body might change to compound
//might also have to store body/compound of attached collider as well

//also have to do some mapping from strings to trigger functions for map loading

//standard events
static void no_event(TRIGGER_ARGS);
static void basic_decide_event(TRIGGER_ARGS);
static void foot_placement(TRIGGER_ARGS);
static void foot_step(TRIGGER_ARGS);
static void grab_event(TRIGGER_ARGS);
static void side_sight_event(TRIGGER_ARGS);
static void main_sight_event(TRIGGER_ARGS);


typedef struct event_struct {
  collider* coll;
  body* body;
  void (*onTrigger)(TRIGGER_ARGS);
  int auto_check;
} event;



#define FUNC_LIM 10
char* func_names[FUNC_LIM];
void (*func_funcs[FUNC_LIM])(TRIGGER_ARGS);

void init_events() {
  null_init_array((void**)func_names, FUNC_LIM);
  null_init_array((void**)func_funcs, FUNC_LIM);
  int i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "no_event";
  func_funcs[i] = no_event;
  i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "basic_decide_event";
  func_funcs[i] = basic_decide_event;
  i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "side_sight";
  func_funcs[i] = side_sight_event;
  i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "main_sight";
  func_funcs[i] = main_sight_event;
  i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "foot_step";
  func_funcs[i] = foot_step;
  i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "grab";
  func_funcs[i] = grab_event;
  
  
}

char* get_event_name(event* e) {
  char* ret = "";
  for (int i = 0; i < FUNC_LIM; i++) {
    if (e->onTrigger == func_funcs[i]) {
      ret = func_names[i];
    }
  }
  return ret;
}

void set_event_by_name(event* e, char* func) {
  int i = char_search(func_names, func, FUNC_LIM);
  if (0 <= i && i < FUNC_LIM) {
    e->onTrigger = func_funcs[i];
  }
}

//things to do in here
//have to take some event shape/range as a poly
//some pointer to virt_pos to center it to
event* make_event(polygon* poly) {
  event* new = malloc(sizeof(event));
  collider* coll = make_collider_from_polygon(poly);
  new->coll = coll;
  new->onTrigger = no_event;
  new->body = NULL;
  new->auto_check = 1;
  return new;
}

void set_auto_check(event* e, int v) {
  e->auto_check = v;
}

int get_auto_check(event* e) {
  return e->auto_check;
}

collider* get_event_collider(event* e) {
  return e->coll;
}

void set_event_body(event* e, body* b) {
  e->body = b;
}

body* get_event_body(event* e) {
  return e->body;
}

void trigger_event(event* e, body* arg, virt_pos* poc) {
  e->onTrigger(e, arg, poc);
}

//this will be some prototype for activating events
void check_events(spatial_hash_map* map, gen_list* e) {
  //will have list be of events in map
  gen_node* curr = e->start;
  event* anEvent;
  while(curr != NULL) {
    anEvent = (event*)(curr->stored);
    if (get_auto_check(anEvent)) {
      check_event(map, anEvent);
    }
    curr = curr->next;
  }
}

void check_event(spatial_hash_map* map, event* e) {
  collider* area = e->coll;
  if (area->collider_node == NULL) {
    collider_ref* cr = make_cr_from_collider(area);
    box cell_shape = get_cell_shape(map);
    set_cr_vectors(area, cr, &cell_shape);
  }
  vector* cells = area->collider_node->active_cells;
  gen_list hits;
  gen_node* aHit = NULL;
  body* hitBody = NULL;
  polygon* eventPoly = get_polygon(get_event_collider(e));
  polygon* bodyPoly = NULL;
  polygon* hitPoly = NULL;
  if (e->body != NULL) {
    bodyPoly = get_polygon(get_collider(e->body));
    virt_pos temp = get_center(bodyPoly);
    set_rotation(eventPoly,get_rotation(bodyPoly));
    set_center(eventPoly, &temp);
  }
  entries_for_collider(map, area, cells);
  initGen_list(&hits);
  store_unique_colliders_in_list(map, cells, &hits);
  aHit = hits.start;
  while(aHit != NULL) {
    hitBody = ((collider*)(aHit->stored))->body;
    hitPoly = get_polygon(get_collider(hitBody));
    vector_2 normal_of_collision = *zero_vec;
    vector_2 b1_norm = *zero_vec;
    vector_2 b2_norm = *zero_vec;
    int actual_collision = find_mtv_of_polygons(eventPoly, hitPoly, &normal_of_collision);
    if (actual_collision != 0) {
      make_unit_vector(&normal_of_collision, &normal_of_collision);
      get_normals_of_collision(eventPoly, hitPoly, &normal_of_collision, &b1_norm, &b2_norm);
      virt_pos poc;
      if (calc_contact_point(eventPoly, hitPoly, &b1_norm, &poc) > 0) {
	//you have point of contact, usefull for limb positioning
	trigger_event(e, hitBody, &poc);
      }
      else {
	trigger_event(e, hitBody, NULL);
      }
    }
    aHit = aHit->next;
  }
  clean_collider_list(&hits);
}



//caller should initialize and clean gen_list before and after calling
//also only finds colliders in same cell as event, have to do fine checking to refine
void store_event_triggers(spatial_hash_map* map, event* e, gen_list* hits) {
  collider* area = e->coll;
  if (area->collider_node == NULL) {
    collider_ref* cr = make_cr_from_collider(area);
    box cell_shape = get_cell_shape(map);
    set_cr_vectors(area, cr, &cell_shape);
  }
  vector* cells = area->collider_node->active_cells;
  entries_for_collider(map, area, cells);
  store_unique_colliders_in_list(map, cells, hits);
}

void no_event(event* e, body* b, virt_pos* poc) {

}

void basic_decide_event(event* e, body* b2, virt_pos* poc) {

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

void main_sight_event(event* e, body* b2, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
  compound* trigger_comp = get_owner(b2);

  smarts* t_comp_sm = get_compound_smarts(trigger_comp);
  smarts* s_comp_sm = get_compound_smarts(self_comp);
  if (t_comp_sm == NULL) {
    return;
  }
  att* trig_atts = get_comp_attributes(t_comp_sm);
  att* self_atts = get_comp_attributes(s_comp_sm);
  vector_2 dir = *zero_vec;
  int run = is_hunter(trig_atts) && is_prey(self_atts);
  int chase = is_hunter(self_atts) && is_prey(trig_atts);
  int pickup  = is_holdable(trig_atts);
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
  //somehow set checks for this
  //could check if trigger mass is inf, or have some attribute for it
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
  compound* trigger_comp = get_owner(trigger);
  
  //somehow set checks for this
  //could check if trigger mass is inf, or have some attribute for it
  if (foreign_body(self, trigger)) {
    jump_action_reset(self_comp);
  }

  //to be triggered when a foot is on ground
  //thinking, add to main body and maybe also foot
  //for foot, add -1 * gravity
  //for body, probably add -1 / 2 * gravity
  //issue is how to determine what is the main body/torso
  //could implicity take head of body part list
}

void grab_event(event* e, body* trigger, virt_pos* poc) {
  body* self = get_event_body(e);
  compound* self_comp = get_owner(self);
  compound* trigger_comp = get_owner(trigger);

  smarts* t_comp_sm = get_compound_smarts(trigger_comp);
  if (t_comp_sm == NULL) {
    return;
  }
  att* trig_atts = get_comp_attributes(t_comp_sm);
  //somehow set checks for this
  //could check if trigger mass is inf, or have some attribute for it
  if (self_comp != trigger_comp) {
    if (is_holdable(trig_atts)) {
      if (get_shared_input(self) != get_shared_input(trigger)) {
	shared_input** si = get_shared_input_ref(trigger);
	virt_pos temp = get_shared_input_origin(*si);
	set_body_center(self, &temp);
	set_shared_input(self, si);
	set_holdable(trig_atts,0);
      }
    }
  }
}
