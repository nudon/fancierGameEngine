#ifndef FILE_MYPARTS_SEEN
#define FILE_MYPARTS_SEEN

#include "events.h"

event* make_basic_vision_event(body* b);
event* make_side_vision_event(body* b);
event* make_main_vision_event(body* b);
event* make_hearing_event(body* b);
event* make_foot_step_event(body* b);
event* make_grab_event(body* b);

//parts
void eyes(body* anchor);
void hand(body* anchor);
void sub_roper(int seg, body* base, tether* param);
void ceiling_roper(int seg, body* base);
void floor_roper(int segments, body* base);

#endif
