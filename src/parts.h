#ifndef FILE_MYPARTS_SEEN
#define FILE_MYPARTS_SEEN
/*
  one of the pre-created object files, generally for creating special body parts or appendages
 */

#include "events.h"

//parts
void eyes(body* anchor);
void hand(body* anchor);
void sub_roper(int seg, body* base, tether* param);
void ceiling_roper(int seg, body* base);
void floor_roper(int segments, body* base);

#endif
