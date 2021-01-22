#ifndef FILE_MYROOMS_SEEN
#define FILE_MYROOMS_SEEN
/*
  originally meant to make defining static maps with code slightly easier, by allowing rooms to be a sub region and allow relative placement of things inside that room. still sort of clunky
 */
#include "compound.h"

#define ROOM_WALL_LEFT 1
#define ROOM_WALL_RIGHT 2
#define ROOM_WALL_TOP 3
#define ROOM_WALL_BOTTOM 4

typedef struct room_struct room;
struct room_struct {
  virt_pos cent;
  int length;
  int height;
};

void init_room(room* r, virt_pos* cent, int len, int height);

//rel len, -1 is left wall of room, +1 is right wall of room
//rel height, -1 is bottom of room, 1 is top of room
virt_pos calc_room_offset(room* r, double rel_len, double rel_height);

void add_compound_to_room(compound* c, room* r, double rel_len, double rel_height);
body* add_wall_to_room(room* r, int wall_type, double rel_start, double rel_end);

#endif
