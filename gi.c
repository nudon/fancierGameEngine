#include <stdlib.h>
#include <stdio.h>
#include "gi.h"
#include "attributes.h"
#include "map.h"
//temp folder for game intelligence stuff

struct event_struct* make_basic_vision_event(body* b) {
  polygon* event_area = vision_triangle(150, 200, 0);
  struct event_struct* e = make_event(event_area);
  set_event_body(e, b);
  set_event(e, basic_decide_event);
  return e;  
}

//visual systmes

//making the cones of vision
//just make an isosolece triangle of base_width and height
//make an event out of it and attach to a body
polygon* vision_triangle(int base, int depth, double rot_off) {
  polygon* tri = createPolygon(3);
  virt_pos point = *zero_pos;
  set_base_point(tri, 0, &point);
  point = (virt_pos){.x = base / 2, .y = -depth};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 1, &point);
  point = (virt_pos){.x = -1 * base / 2, .y = -depth};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 2, &point);
  generate_normals_for_polygon(tri);
  return tri;
}


void basic_decide_event(struct event_struct* e, body* b2) {
  body* b1 = get_event_body(e);
  if (b1 != NULL) {
    basic_decide(b1, b2);
  }
}

//decision process
//think its better to set a direction in which to move than a position
void basic_decide(body* self, body* triger) {
  //might have some vector_2 as a reference argument to set
  compound* self_comp = get_owner(self);
  compound* triger_comp = get_owner(triger);
  decision_att* sa = get_attributes(self_comp);
  decision_att* ta = get_attributes(triger_comp);
  virt_pos self_center = *zero_pos;
  virt_pos triger_center = *zero_pos;
  vector_2 dir = *zero_vec;
  vector_2 temp = *zero_vec;
  double alpha = 0.5;
  printf("hewwo\n");
  if (is_hunter(ta) && is_prey(sa)) {
    //then run away
    self_center = *getCenter(self);
    triger_center = *getCenter(triger);
    vector_between_points(&triger_center, &self_center, &dir);
    temp = get_dir(get_owner(self));
    exponential_decay_vector(&temp, &dir, &temp, alpha);
    set_dir(get_owner(self), &temp);
    printf("noot noot time to scoot\n");
  }
}

//movement to dir is handled by poltergeists
