#ifndef FILE_EVENTS_DEFINED
#define FILE_EVENTS_DEFINED

typedef struct event_struct event;

#include "body.h"
#include "collider.h"
#include "geometry.h"


#define TRIGGER_ARGS event* e, body* b, virt_pos* poc

void init_events();
char* get_event_name(event* e);
void set_event_by_name(event* e, char* func);

event* make_event(polygon* poly);
void set_auto_check(event* e, int v);

collider* get_event_collider(event* e);
void set_event_body(event* e, body* b);
body* get_event_body(event* e);

void trigger_event(TRIGGER_ARGS);

//this will be some prototype for activating events
void check_events(spatial_hash_map* map, gen_list* e);
void check_event(spatial_hash_map* map, event* e);
//caller should initialize and clean gen_list before and after calling
//also only finds colliders in same cell as event, have to do fine checking to refine
void store_event_triggers(spatial_hash_map* map, event* e, gen_list* hits);

#endif
