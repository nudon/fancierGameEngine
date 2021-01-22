#include "room.h"
#include "shapes.h"

void init_room(room* r, virt_pos* cent, int len, int height) {
  r->cent = *cent;
  r->length = len;
  r->height = height;
}

//rel len, -1 is left wall of room, +1 is right wall of room
//rel height, -1 is bottom of room, 1 is top of room
virt_pos calc_room_offset(room* r, double rel_len, double rel_height) {
  virt_pos offset = r->cent;
  offset.x += r->length * 0.5 * rel_len;
  offset.y -= r->height * 0.5 * rel_height;
  return offset;
}

void add_compound_to_room(compound* c, room* r, double rel_len, double rel_height) {
  virt_pos offset = calc_room_offset(r, rel_len, rel_height);
  offset_compound(c, &offset);
}



body* add_wall_to_room(room* r, int wall_type, double rel_start, double rel_end) {
  body* wall = NULL;
  int width = -1, height = -1;
  virt_pos cent = *zero_pos;
  if (wall_type == ROOM_WALL_LEFT || wall_type == ROOM_WALL_RIGHT) {
    width = M_W;
    height = (abs(rel_start) + abs(rel_end)) * 0.5 * r->height;
    wall = quick_block(width, height, DEF_FN);
    if (wall_type == ROOM_WALL_LEFT) {
      cent = calc_room_offset(r, -1, (rel_start + rel_end) * 0.5);
      //cent.x -= width  * 0.5;
    }
    else {
      cent = calc_room_offset(r, 1, (rel_start + rel_end) * 0.5);
      //cent.x += width * 0.5 ;
    }
    offset_body(wall, &cent);
  }
  else if (wall_type == ROOM_WALL_TOP || wall_type == ROOM_WALL_BOTTOM) {
    width = (abs(rel_start) + abs(rel_end)) * 0.5 * r->length;
    height = M_H;
    wall = quick_block(width, height, DEF_FN);
    if (wall_type == ROOM_WALL_TOP) {
      cent = calc_room_offset(r, (rel_start + rel_end) * 0.5, 1);
      //cent.y += height * 0.5;
    }
    else {
      cent = calc_room_offset(r, (rel_start + rel_end) * 0.5, -1);
      //cent.y -= height * 0.5;
    }
    offset_body(wall, &cent);
  }
  return wall;
}
