#ifndef FILE_GI_SEEN
#define FILE_GI_SEEN
#include "events.h"
#include "body.h"
#include "collider.h"

event* make_basic_vision_event(body* b);

polygon* vision_triangle(int base, int depth, double rot_off);

void basic_decide_event(struct event_struct* e, body* b2);

void basic_decide(body* self, body* triger);

#endif
