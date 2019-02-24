#include "events.h"


//so, events

//basically, lots of similarity with regular collision detection
//only difference is instead of doing collision, you call some arbitrary function
//

//side note, thinking of changing the center in polygons to be a pointer.
//would make some bounding_box code easier,
//otherwise will have to have some virt_pos* in event to manually update the event coll. 
//might also have some bitmask or validation function
//onTrigger might have a different method signature latter. body might change to compound
typedef struct event_struct {
  collider* coll;
  
  void (*onTrigger)(struct event_struct* e1, body* e2);
} event;


void printEvent(struct event_struct* not, body* used) {
  fprintf(stderr, "you triggered an event!\n Event addr is %lu, body addr is %lu\n",
	  (unsigned long)not, (unsigned long)used);
}

//things to do in here
//have to take some event shape/range as a poly
//some pointer to virt_pos to center it to
//probably also 
event* make_event(box* cell_shape, polygon* poly, virt_pos* event_origin) {
  event* new = malloc(sizeof(event));
  //important to reassign polys center before creating collider
  free(poly->center);
  poly->center = event_origin;
  collider* coll = make_collider_from_polygon(poly);
  collider_list_node* cln = make_cln_from_collider(coll);
  set_cln_vectors(coll, cln, cell_shape);
  new->coll = coll;
  new->onTrigger = printEvent;
  return new;
}



//this will be some prototype for activating events
void check_events(spatial_hash_map* map, gen_list* e) {
  //will have list be of events in map
  gen_node* curr = e->start;
  gen_node* aHit;
  body* hitBody;
  event* anEvent;
  collider* area;
  vector* cells;
  gen_list hits;
  while(curr != NULL) {
    anEvent = (event*)(curr->stored);
    area = anEvent->coll;
    //event collider never got inserted into map, so collider_node never got malloced
    
    cells = area->collider_node->active_cells;
    entries_for_collider(map, area, cells);
    initGen_list(&hits);
    store_unique_colliders_in_list(map, cells, &hits);
    //can iterate over hits, checking for collision,
    aHit = hits.start;
    while(aHit != NULL) {
      hitBody = ((collider*)(aHit->stored))->body;
      anEvent->onTrigger(anEvent, hitBody);
      aHit = aHit->next;
    }
    clean_collider_list(&hits);
    curr = curr->next;
  }
}
