#include <stdio.h>
#include "events.h"
#include "util.h"
#include "guts.h"

static void no_event(TRIGGER_ARGS);

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
  char* ret = NULL;
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

void check_events(spatial_hash_map* map, gen_list* e) {
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

void init_event_collider(spatial_hash_map* map, event* e) {
  collider* area = get_event_collider(e);
  collider_ref* cr = make_cr_from_collider(area);
  box cell_shape = get_cell_shape(map);
  set_cr_vectors(area, cr, &cell_shape);
}

void clear_event_collider(event* e) {
  collider* area = get_event_collider(e);
  collider_ref* cr = get_collider_ref(area);
  free_cr(cr);
}

void check_event(spatial_hash_map* map, event* e) {
  collider* area = e->coll;
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
  init_gen_list(&hits);
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


