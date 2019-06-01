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

typedef struct event_struct {
  collider* coll;
  body* associated_body;
  void (*onTrigger)( event* e1, body* e2);
} event;


#define FUNC_LIM 10
char* func_names[FUNC_LIM];
void (*func_funcs[FUNC_LIM])(event* e, body* b);

void init_events() {
  null_init_array((void**)func_names, FUNC_LIM);
  null_init_array((void**)func_funcs, FUNC_LIM);
  int i = first_empty_index(func_names, FUNC_LIM);
  func_names[i] = "no_event";
  func_funcs[i] = no_event;
  
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

void printEvent(struct event_struct* not, body* used) {
  fprintf(stderr, "you triggered an event!\n Event addr is %lu, body addr is %lu\n",
	  (unsigned long)not, (unsigned long)used);
}

//things to do in here
//have to take some event shape/range as a poly
//some pointer to virt_pos to center it to
event* make_event(polygon* poly) {
  event* new = malloc(sizeof(event));
  collider* coll = make_collider_from_polygon(poly);
  new->coll = coll;
  new->onTrigger = no_event;
  new->associated_body = NULL;
  return new;
}

void set_event(event* e, void (*newTrigger)(struct event_struct* e, body* b)) {
  e->onTrigger = newTrigger;
}

void set_event_body(event* e, body* b) {
  virt_pos* cent = get_polygon(e->coll)->center;
  free(cent);
  get_polygon(e->coll)->center = get_polygon(get_collider(b))->center;
  e->associated_body = b;
}

body* get_event_body(event* e) {
  return e->associated_body;
}

collider* get_event_collider(event* e) {
  return e->coll;
}

void trigger_event(event* e, body* arg) {
  e->onTrigger(e, arg);
}

//this will be some prototype for activating events
void check_events(spatial_hash_map* map, gen_list* e) {
  //will have list be of events in map
  gen_node* curr = e->start;
  event* anEvent;
  while(curr != NULL) {
    anEvent = (event*)(curr->stored);
    check_event(map, anEvent);
    curr = curr->next;
  }
}

void check_event(spatial_hash_map* map, event* e) {
  collider* area = e->coll;
  if (area->collider_node == NULL) {
    collider_list_node* cln = make_cln_from_collider(area);
    box cell_shape = get_cell_shape(map);
    set_cln_vectors(area, cln, &cell_shape);
  }
  vector* cells = area->collider_node->active_cells;
  gen_list hits;
  gen_node* aHit = NULL;
  body* hitBody = NULL;
  polygon* eventPoly = get_polygon(get_event_collider(e));
  entries_for_collider(map, area, cells);
  initGen_list(&hits);
  store_unique_colliders_in_list(map, cells, &hits);
  aHit = hits.start;
  while(aHit != NULL) {
    hitBody = ((collider*)(aHit->stored))->body;
    if (do_polygons_intersect(get_polygon(get_collider(hitBody)), eventPoly)) {
      trigger_event(e, hitBody);
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
    collider_list_node* cln = make_cln_from_collider(area);
    box cell_shape = get_cell_shape(map);
    set_cln_vectors(area, cln, &cell_shape);
  }
  vector* cells = area->collider_node->active_cells;
  entries_for_collider(map, area, cells);
  store_unique_colliders_in_list(map, cells, hits);
}

//standard event
void no_event(event* e, body* b) {

}
