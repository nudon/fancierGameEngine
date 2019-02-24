#ifndef FILE_EVENTS_DEFINED
#define FILE_EVENTS_DEFINED

#include "body.h"
#include "collider.h"
#include "geometry.h"

struct event_struct;

struct event_struct* make_event(box* cell_shape, polygon* poly, virt_pos* event_origin);



//this will be some prototype for activating events
void check_events(spatial_hash_map* map, gen_list* e);


#endif
