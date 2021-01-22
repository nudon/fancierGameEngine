#ifndef FILE_EVENTS_DEFINED
#define FILE_EVENTS_DEFINED

/*
  sort of a special collision object that has a collider, but does not get inserted into any maps, but purely used to queary which bodies are within it's boundaries, and call some sort of handling function on each one, which likely has some filter for what type of body it applies to
  
  has a large repeat of collision logic, which is sort of awkward to fix. Events aren't actually inserted into shm, they just exist somewhere and have a collider to grab overlaps from the map, and combining that funcionality with the general collision system might be a bit weird to do. 

*/

typedef struct event_struct event;

#include "body.h"
#include "collider.h"
#include "geometry.h"


#define TRIGGER_ARGS event* e, body* b, virt_pos* poc

//inits internal name-funct parallell array
void init_events();

//gets name of event, or sets event to have func associated with name
char* get_event_name(event* e);
void set_event_by_name(event* e, char* func);

//makes a blank event with shape of poly
event* make_event(polygon* poly);

//getter/setter for auto-check, which is automatically checking the event within check_events
int get_auto_check(event* e);
void set_auto_check(event* e, int v);

//getter/setter for fields
collider* get_event_collider(event* e);
void set_event_body(event* e, body* b);
body* get_event_body(event* e);

//runs event logic 
void trigger_event(TRIGGER_ARGS);

void init_event_collider(spatial_hash_map* map, event* e);
void clear_event_collider(event* e);

//checks all auto-events within e on map
void check_events(spatial_hash_map* map, gen_list* e);

//has a copy-past of collision-checking for event, to run custom on-trigger logic
void check_event(spatial_hash_map* map, event* e);

//caller should initialize and clean gen_list before and after calling
//also only finds colliders in same cell as event, have to do fine checking to refine
void store_event_triggers(spatial_hash_map* map, event* e, gen_list* hits);

#endif
