#include "shapes.h"
//handles initial setup
body* blankBody(polygon* base) {
  body* b = NULL;
  collider* c = make_collider_from_polygon(base);
  fizzle* f = createFizzle();
  init_fizzle(f);
  b = createBody(f, c);
  return b;
}

body* makeNormalBody(int sides, double scale) {
  polygon* ngon = NULL;
  body* b;
  ngon = createNormalPolygon(sides);
  set_scale(ngon, scale);
  b = blankBody(ngon);
  return b;
}

body* makeRectangleBody(int width, int height) {
  polygon* rect = createRectangle(width, height);
  body* b = blankBody(rect);
  return b;
}

body* quick_nopic_block(int width, int height) {
  body* b = makeBlock(width, height);
  return b;
}

body* quick_block(int width, int height, char* fn) {
  body* b = quick_nopic_block(width, height);
  set_picture_by_name(b, fn);
  return b;
}

body* quick_tile_block(int width, int height, char* fn) {
  body* b = quick_nopic_block(width, height);
  tile_texture_for_body(b, fn, 3,3,0,0);
  return b;
}
  


//functions for automatically creating/spacing multiple objects
#define HORZ_CHAIN 0
#define VERT_CHAIN 1
 
compound* makeBlockChain(int pos_x, int pos_y, int width, int height, char* pic_fn, int len, int chain_type) {
  vector_2 dir;
  double disp = 0;
  if (chain_type == HORZ_CHAIN) {
    dir = (vector_2){.v1 = 1, .v2 = 0};
    disp = width - 3;
  }
  else if (chain_type == VERT_CHAIN) {
    dir = (vector_2){.v1 = 0, .v2 = 1};
    disp = height;
  }
  else {
    fprintf(stderr, "unset chain dir\n");
  }
  body* base_block = makeBlock(width, height);
  set_picture_by_name(base_block, pic_fn);
  virt_pos center = (virt_pos){.x = pos_x, .y = pos_y};
  return makeBodyChain(base_block, &center, len, &dir, disp);
}
 
// duplicates a body in the specified way and returns compound, original body is not used
//also makes compound uniform, meaning bodies rotate and move in sync
compound* makeBodyChain(body* start, virt_pos* start_pos,  int len, vector_2* dir, double disp) { 
  compound* chain = create_compound();
  body* temp = NULL;
  polygon* p = NULL;
  make_unit_vector(dir, dir);
  vector_2 temp_vec = *zero_vec;
  virt_pos disp_pos = *zero_pos;
  for (int i = 0; i < len; i++) {
    temp = cloneBody(start);
    p = get_polygon(get_collider(temp));
    vector_2_scale(dir, disp * i, &temp_vec);
    vector_2_to_virt_pos(&temp_vec, &disp_pos);
    virt_pos_add(&disp_pos, start_pos, &disp_pos);
    add_offset_to_center(p, &disp_pos);
    add_body_to_compound(chain, temp);
  }
  return chain;
}

//making the cones of vision
//just make an isosolece triangle of base_width and height
//make an event out of it and attach to a body
polygon* vision_cone(int radius, double theta_deg, int steps, double rot_off) {
  double theta = theta_deg * DEG_2_RAD;
  if (theta > M_PI) {
    fprintf(stderr, "warning, theta for vision cone would be concave, capping it to a cemi-circle\n");
    theta = M_PI;
  }
  polygon* cone = createPolygon(steps + 1);
  virt_pos point = *zero_pos;
  double theta_step = theta / steps;
  rot_off -= theta / 2.0;
  set_base_point(cone, 0, &point);
  for (int i = 0; i < steps; i++) {
    point = (virt_pos){.x = radius, .y = 0};
    virt_pos_rotate(&point, i * theta_step + rot_off, &point);
    set_base_point(cone, i + 1, &point);
  }
  generate_normals_for_polygon(cone);
  return cone;
  
}

polygon* vision_triangle(int base, int depth, double rot_off) {
  polygon* tri = createPolygon(3);
  virt_pos point = *zero_pos;
  set_base_point(tri, 0, &point);
  point = (virt_pos){.x = depth, .y = base / 2};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 1, &point);
  point = (virt_pos){.x = depth, .y = -base / 2};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 2, &point);
  generate_normals_for_polygon(tri);
  return tri;
}

//just some standard map transitions, 
polygon* make_event_poly(polygon* shape) {
  polygon* new = createNormalPolygon(12);
  return new;
}


//blocks are just rectangles are meant to be used as floor/wall tiles
body* makeBlock (int width, int height) {
  body* b = NULL;
  polygon* p = createRectangle(width, height);
  b = blankBody(p);
  fizzle* f = get_fizzle(b);
  set_mass(f, INFINITY);
  set_moi(f, INFINITY);
  set_bounce(f, 0.1);
  return b;
}
