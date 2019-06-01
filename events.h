#ifndef FILE_EVENTS_DEFINED
#define FILE_EVENTS_DEFINED

#include "body.h"
#include "collider.h"
#include "geometry.h"

typedef struct event_struct event;


void init_events();
char* get_event_name(event* e);
void set_event_by_name(event* e, char* func);

void printEvent(struct event_struct* not, body* used);
event* make_event(polygon* poly);
void set_event(event* e, void (*newTrigger)(struct event_struct* e, body* b));
void set_event_body(event* e, body* b);
body* get_event_body(event* e);
collider* get_event_collider(event* e);

void trigger_event(event* e, body* arg);

//this will be some prototype for activating events
void check_events(spatial_hash_map* map, gen_list* e);
void check_event(spatial_hash_map* map, event* e);
//caller should initialize and clean gen_list before and after calling
//also only finds colliders in same cell as event, have to do fine checking to refine
void store_event_triggers(spatial_hash_map* map, event* e, gen_list* hits);

//standard events
void no_event(event* e, body* b);

#endif
