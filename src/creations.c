//sort of behaves like a collection of prefabs

#include <stdlib.h>
#include <stdio.h>

#include "creations.h"
#include "graphics.h"
#include "gi.h"
#include "map_io.h"
#include "sizes.h"
#include "names.h"
body* quick_nopic_block(int width, int height);
body* quick_block(int width, int height, char* fn);
body* quick_tile_block(int width, int height, char* fn);

void eyes(body* anchor);
void hand(body* anchor);

#define MAP_DIR "./maps/"
char* ORIGIN_MAP_NAME = MAP_DIR"origin.map";
char* BEACH_MAP_NAME = MAP_DIR"beach.map";
char* BASIC_MAP_NAME = MAP_DIR"basic.map";



virt_pos ORIGIN_BEACH_POS = (virt_pos){.x= 345, .y=270};

typedef struct room_struct room;
struct room_struct {
  virt_pos cent;
  int length;
  int height;
};

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

void offset_body(body* b, virt_pos* offset) {
  virt_pos cent;
  cent = get_body_center(b);
  virt_pos_add(&cent, offset, &cent);
  set_body_center(b, &cent);
}

void offset_compound(compound* c, virt_pos* offset) {
  gen_node* curr = get_bodies(c)->start;
  body* b = NULL;
  while(curr != NULL) {
    b = (body*)curr->stored;
    offset_body(b, offset);
    
    curr = curr->next;
  }
}

void add_compound_to_room(compound* c, room* r, double rel_len, double rel_height) {
  virt_pos offset = calc_room_offset(r, rel_len, rel_height);
  offset_compound(c, &offset);
}

#define ROOM_WALL_LEFT 1
#define ROOM_WALL_RIGHT 2
#define ROOM_WALL_TOP 3
#define ROOM_WALL_BOTTOM 4


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

event* make_basic_vision_event(body* b) {
  polygon* event_area = vision_triangle(150, 200, 0);
  event* e = make_event(event_area);
  set_event_by_name(e, "basic_decide_event");
  add_event_to_body(b,e);
  return e;  
}

//side vision, event should prioritize fast moving things, result is rotating body towards thing
event* make_side_vision_event(body* b) {
  polygon* event_area = vision_cone(150, 110, 5, 0);
  event* e = make_event(event_area);
  set_event_by_name(e, "side_sight");
  add_event_to_body(b,e);
  return e;  
}

//main vision, event should prioritize the closest thing in range, result is anaylzing thing in range and ?
event* make_main_vision_event(body* b) {
  polygon* event_area = vision_cone(220, 20, 2, 0);
  event* e = make_event(event_area);
  set_event_by_name(e, "main_sight");
  add_event_to_body(b,e);
  return e;  
}

//hearing, should behave like side vision by noticing things that are moving quickly
event* make_hearing_event(body* b) {
  polygon* event_area = createNormalPolygon(9);
  set_scale(event_area, 20);
  event* e = make_event(event_area);
  set_event_by_name(e, "side_sight");
  add_event_to_body(b,e);
  return e;  
}

event* make_foot_step_event(body* b) {
  polygon* area = clonePolygon(get_polygon(get_collider(b)));
  event* e = make_event(area);
  set_event_by_name(e, "foot_step");
  add_event_to_body(b,e);
  return e;
}

event* make_grab_event(body* b) {
  polygon* area = clonePolygon(get_polygon(get_collider(b)));
  set_scale(area, get_scale(area) + 3);
  event* e = make_event(area);
  set_event_by_name(e, "grab");
  add_event_to_body(b,e);
  set_auto_check(e, 0);
  return e;
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

struct compound_spawner_struct {
  char* compound_name;
  int spawn_cap;
  virt_pos spawn_center;
  compound* (*spawner) (void);
};

char* get_spawner_name(compound_spawner* spawn) {
  return spawn->compound_name;
}

int get_spawner_cap(compound_spawner* spawn) {
  return spawn->spawn_cap;
}

void get_spawner_pos(compound_spawner* spawn, virt_pos* result) {
  *result = spawn->spawn_center;
}

compound_spawner* create_compound_spawner(char* name, int cap, int x_pos, int y_pos) {
  compound_spawner* spawn = malloc(sizeof(compound_spawner));
  spawn->compound_name = strdup(name);
  spawn->spawn_cap = cap;
  spawn->spawn_center = (virt_pos){.x = x_pos, .y = y_pos};
  //then look at name and find / set spawner func
  if (strcmp(name, BLUE_SLIME_SPAWN) == 0) {
    spawn->spawner = &makeSlime;
  }
  else if (strcmp(name, TEST_SPAWN) == 0) {
    //spawn->spawner = &tunctish;
    spawn->spawner = &monkey;
  }
  else if (strcmp(name, TRASHCAN_SPAWN) == 0) {
    spawn->spawner = &makeTrashCan;
  }
  else if (strcmp(name, GOHEI_SPAWN) == 0) {
    spawn->spawner = &makeGohei;
  }
  else {
    fprintf(stderr, "warning, unable to find spawner for %s\n", name);
  }
  return spawn;
}

void trigger_spawners_in_map(map* map) {
  gen_node* curr_plane = get_planes(map)->start;
  gen_node* curr_spawner;
  plane* p = NULL;
  compound_spawner* spawn = NULL;
  while(curr_plane != NULL) {
    p = (plane*)curr_plane->stored;
    curr_spawner = get_spawners(p)->start;
    while(curr_spawner != NULL) {
      spawn = (compound_spawner*)curr_spawner->stored;
      trigger_spawner(spawn, p);
      curr_spawner = curr_spawner->next;
    }
    curr_plane = curr_plane->next;
  }
}

void trigger_spawner(compound_spawner* spawn, plane* insert) {
  virt_pos pos = *zero_pos;
  compound* spawned = NULL;
  if (spawn->spawn_cap != 0) {
    if (spawn->spawn_cap > 0) {
      spawn->spawn_cap--;
    }
    get_spawner_pos(spawn, &pos);
    spawned = spawn->spawner();
    offset_compound(spawned, &pos);
    add_compound_to_plane(insert, spawned);
  }
}



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

//loadzones are still pretty annoying to create
void insert_load_zone_into_plane(char* from_map, char* to_map, plane* from_plane, char* to_plane_name, virt_pos* from_pos, virt_pos* to_pos) {
  char* from_plane_name = get_plane_name(from_plane);
  event* load_event = make_load_event(from_pos);
  load_zone* lz = make_load_zone(from_map, to_map, from_plane_name, to_plane_name, to_pos, load_event);
  add_load_zone_to_plane(from_plane, lz);
}

event* make_load_event(virt_pos* cent) {
  polygon* poly = createNormalPolygon(7);
  set_center(poly, cent);
  event* load = make_event(poly);
  return load;
}

/// prefabs

//walls make walls that completely surround the visual edges of screen
compound* makeWalls() {
  compound* wallCompound = create_compound();
  body* b = NULL;
  polygon* aWall = NULL;
  double wallMass = INFINITY;
  int xVal, yVal;
  int w, h;
  virt_pos temp = *zero_pos;
  for(int i = 0; i < 4; i++) {
    if (i % 2 == 0) {
      h = getScreenHeight() / 2;
      w =  getScreenWidth() / 8;
      
      yVal = getScreenHeight() / 2;
      xVal = getScreenWidth();
      if (i == 2) {
	xVal = 0;
      }
    }
    else {
      h = getScreenHeight() / 8;
      w =  getScreenWidth() / 2;
      xVal = getScreenWidth() / 2;
      yVal = getScreenHeight();
      if (i == 3) {
	yVal = 0;
      }
    }
    aWall = createRectangle(w,h);
    temp.x = xVal;
    temp.y = yVal;
    set_center(aWall, &temp);
    generate_normals_for_polygon(aWall);

    b = blankBody(aWall);
    set_mass(get_fizzle(b), wallMass);
    add_body_to_compound(wallCompound, b);
  }
  return wallCompound;
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

compound* makeCentipede(int segments) {
  compound* centComp = create_compound();
  body* body;
  virt_pos* center = &(virt_pos){.x = 0, .y = 0};
  for (int i = 0; i < segments; i++) {
    body = makeNormalBody(8, 2);
    set_center(get_polygon(get_collider(body)), center);
    add_body_to_compound(centComp, body);
  }
  tether_join_compound(centComp, NULL);
  return centComp;
}

compound* makeCrab() {
  compound* centComp = create_compound();
  add_smarts_to_comp(centComp);
  smarts* sm = get_compound_smarts(centComp);
  att* bits = get_comp_attributes(sm);
  polygon* poly;
  collider* coll;
  fizzle* fizz;
  body* body;
  virt_pos* center = &(virt_pos){.x = 0, .y = 0};
  poly = createRectangle(90, 60);

  set_center(poly, center);
  coll = make_collider_from_polygon(poly);
  
  fizz = createFizzle();
  init_fizzle(fizz);
  set_bounce(fizz, 0.3);
  set_gravity(fizz, g);
  body = createBody(fizz, coll);
  make_foot_step_event(body);
  add_body_to_compound(centComp, body);

  set_picture_by_name(body, CRAB_FN );

  set_prey(bits, 1);
  set_hunter(bits, 1);

  return centComp;
}

compound* makeSlime() {
  compound* centComp = create_compound();
  add_smarts_to_comp(centComp);
  polygon* poly;
  collider* coll;
  fizzle* fizz;
  body* slime_body;
  virt_pos* center = &(virt_pos){.x = 0, .y = 0};
  poly = createRectangle(48, 48);

  set_center(poly, center);
  coll = make_collider_from_polygon(poly);
  
  fizz = createFizzle();
  init_fizzle(fizz);
  set_bounce(fizz, 3.3);
  set_gravity(fizz, g);
  slime_body = createBody(fizz, coll);
  add_body_to_compound(centComp, slime_body);

  set_picture_by_name(slime_body, BLUE_SLIME_FN );

  poltergeist* p = make_poltergeist();
  give_standard_poltergeist(p);
  set_poltergeist(slime_body, p);
  //set_hunter(get_attributes(centComp), 1);
  make_basic_vision_event(slime_body);
  set_contact_damage(slime_body, 5);

  return centComp;
}

compound* tunctish() {
  compound* comp = create_compound();
  add_smarts_to_comp(comp);
  shared_input** torso_si = create_shared_input_ref();
  virt_pos center = (virt_pos){.x = 0, .y = 0};
  
  body* torso = makeNormalBody(13,7);

  make_foot_step_event(torso);

  set_body_center(torso, &center);
  
  poltergeist* torso_polt = make_poltergeist();
  set_polt_by_name(torso_polt, "bb_polt");
  set_poltergeist(torso, torso_polt);

  tile_texture_for_body(torso, DEF_FN, 6,6,0,0);
  
  set_shared_input_origin(*torso_si, torso, SI_CENTER);
  
  set_shared_input(torso, torso_si);
  
  add_body_to_compound(comp, torso);

  eyes(torso);
  hand(torso);
  
  set_compound_gravity(comp, g);


  set_hunter(get_comp_attributes(get_compound_smarts(comp)), 1);
  return comp;
}

compound* monkey() {
  compound* comp = create_compound();
  add_smarts_to_comp(comp);
  shared_input** torso_si = create_shared_input_ref();
  virt_pos center = (virt_pos){.x = 0, .y = 0};
  
  body* torso = makeNormalBody(13,4);

  make_foot_step_event(torso);
  
  set_body_center(torso, &center);

  poltergeist* torso_polt = make_poltergeist();
  set_polt_by_name(torso_polt, "bb_polt");
  set_poltergeist(torso, torso_polt);
  
  tile_texture_for_body(torso, DEF_FN, 6,6,0,0);
  
  set_shared_input_origin(*torso_si, torso, SI_CENTER);
  
  set_shared_input(torso, torso_si);
  add_body_to_compound(comp, torso);

  eyes(torso);
  hand(torso);
  
  set_compound_gravity(comp, g);


  set_hunter(get_comp_attributes(get_compound_smarts(comp)), 1);
  set_prey(get_comp_attributes(get_compound_smarts(comp)), 1);
  return comp;
}

//parts
void eyes(body* anchor) {
  compound* comp = get_owner(anchor);
  shared_input** si_ref = get_shared_input_ref(anchor);
  
  body* left_eye_anchor = makeNormalBody(3,1);
  body* right_eye_anchor = makeNormalBody(3,1);
  body* left_eye = makeNormalBody(9, 2);
  body* right_eye = makeNormalBody(9, 2);
  
  make_side_vision_event(left_eye);
  make_side_vision_event(right_eye);

  make_main_vision_event(left_eye);
  make_main_vision_event(right_eye);
  
  int anchor_width = get_bb_width(get_collider(anchor));
  int anchor_height = get_bb_height(get_collider(anchor));
  
  virt_pos left_eye_p, right_eye_p;
  polygon* a_poly = get_polygon(get_collider(anchor));
  //set_rotation(a_poly, 0);
  
  left_eye_p = get_center(a_poly);
  right_eye_p = get_center(a_poly);
  virt_pos eye_offset = (virt_pos){.x = anchor_width * -0.5 * .75,
				   .y = anchor_height * -0.5 * 0.2};
  virt_pos_add(&eye_offset, &left_eye_p, &left_eye_p);
  eye_offset.x *= -1;
  virt_pos_add(&eye_offset, &right_eye_p, &right_eye_p);

  set_body_center(left_eye_anchor, &left_eye_p);
  set_body_center(right_eye_anchor, &right_eye_p);
  set_body_center(left_eye, &left_eye_p);
  set_body_center(right_eye, &right_eye_p);
  
  poltergeist* eye_polt = make_poltergeist();
  set_polt_by_name(eye_polt, "look_polt");
  set_poltergeist(left_eye, eye_polt);
  set_poltergeist(right_eye, eye_polt);
  
  tether* left_eye_tether = tether_bodies(left_eye_anchor, left_eye, one_way_tether);
  tether* right_eye_tether = tether_bodies(right_eye_anchor, right_eye, one_way_tether);
    
  add_tether_to_compound(comp, left_eye_tether);
  add_tether_to_compound(comp, right_eye_tether);
  
  tile_texture_for_body(left_eye, EYE_FN, 3,3,0,0);
  tile_texture_for_body(right_eye, EYE_FN, 3,3,0,0);
  
  set_shared_input(left_eye_anchor, si_ref);
  set_shared_input(right_eye_anchor, si_ref);

  add_body_to_compound(comp, left_eye_anchor);
  //add_body_to_compound(comp, right_eye_anchor);
  
  add_body_to_compound(comp, left_eye);
  //add_body_to_compound(comp, right_eye);

  return;
}

void hand(body* anchor) {
  compound* comp = get_owner(anchor);
  body* hand = makeNormalBody(3,3);
  make_grab_event(hand);
  
  poltergeist* hand_polt = make_poltergeist();
  set_polt_by_name(hand_polt, "hand_polt");
  set_poltergeist(hand, hand_polt);
  
  tether* hand_tether = tether_bodies(anchor, hand, one_way_tether);
  set_tether_k(hand_tether, .08);
  add_tether_to_compound(comp, hand_tether);
  
  tile_texture_for_body(hand, DEF_FN, 3,3,0,0);
  
  add_body_to_compound(comp, hand);
  
  return;
}
////




compound* makeTrashCan() {
  compound* can = create_compound();
  virt_pos* center = &(virt_pos){.x = 0, .y = 0};
  shared_input** si = create_shared_input_ref();
  body* bottom = NULL;
  body* lside = NULL;
  body* rside = NULL;
  polygon* bp = NULL, *lp = NULL, *rp = NULL;
  virt_pos temp_cent; 
  int width = 5;
  int bl = 80;
  int sl = 150;
  double side_theta = 80 * DEG_2_RAD;
  double side_omega = 180 * DEG_2_RAD - side_theta;
  double a, b;
  //printf("%f and %f\n", side_theta, side_omega);
  bp = createRectangle(bl, width);
  lp = createRectangle(sl, width);
  rp = createRectangle(sl, width);
  set_rotation(lp, side_theta);
  set_rotation(rp, side_omega);
  
  //need to rotate and offset polygons
  set_center(bp, center);

  a = cos(side_omega) * (sl / 2);
  b = sin(side_omega) * (sl / 2);
  temp_cent.x = (bl / 2) - a;
  temp_cent.y = -b;
  virt_pos_add(center, &temp_cent, &temp_cent);
  set_center(rp, &temp_cent);
  temp_cent.x = ((bl / 2) - a) * -1;
  temp_cent.y = -b;
  virt_pos_add(center, &temp_cent, &temp_cent);
  set_center(lp, &temp_cent);

  bottom = blankBody(bp);
  lside = blankBody(lp);
  rside = blankBody(rp);
  set_shared_input_origin(*si, bottom, SI_CENTER);
  set_shared_input(bottom, si);
  set_shared_input(lside, si);
  set_shared_input(rside, si);
  add_body_to_compound(can, bottom);
  add_body_to_compound(can, lside);
  add_body_to_compound(can, rside);
  set_compound_gravity(can, g);
  return can;
}

compound* makeGohei() {
  compound* gohei = create_compound();
  add_smarts_to_comp(gohei);
  virt_pos* center = &(virt_pos){.x = 0, .y = 0};
  shared_input** si = create_shared_input_ref();
  body* handle, *left_side, *right_side;
  int handle_width = 6;
  int handle_height = 70;
  int side_width = 15;
  handle = makeRectangleBody(handle_width,handle_height);
  virt_pos offset = (virt_pos){.x = handle_width / 2, -handle_height / 2};
  right_side = makeRectangleBody(side_width,side_width);
  left_side = makeRectangleBody(side_width,side_width);
  set_body_center(right_side, &offset);
  offset.x *= -1;
  set_body_center(left_side, &offset);
  set_shared_input_origin(*si, handle, SI_CENTER);
  set_shared_input(handle, si);
  set_shared_input(right_side, si);
  add_body_to_compound(gohei,handle);
  add_body_to_compound(gohei,right_side);
  set_compound_gravity(gohei, g);
  
  smarts* sm = get_compound_smarts(gohei);
  att* c_atts = get_comp_attributes(sm);
  set_holdable(c_atts, 1);

  return gohei;
}


/// special things

void make_compound_user(compound* comp) {
  body* head = (body*)get_bodies(comp)->start->stored;
  poltergeist* polt = make_poltergeist();
  smarts * sm = get_compound_smarts(comp);
  att* comp_att = get_comp_attributes(sm);
  give_user_poltergeist(polt);
  set_poltergeist(head, polt);
  set_user(comp_att, 1);
  set_travel(comp_att, 1);
  center_cam_on_body(head);
}

//just some standard map transitions, 
polygon* make_event_poly(polygon* shape) {
  polygon* new = createNormalPolygon(12);

  return new;
}

void write_maps_to_disk() {
  map* aMap = NULL;
  FILE* mapFile = NULL;
  
  aMap = make_beach_map();
  mapFile = fopen(BEACH_MAP_NAME, "w+");
  xml_write_map(mapFile, aMap);
  fclose(mapFile);

  aMap = make_basic_map();
  mapFile = fopen(BASIC_MAP_NAME, "w+");
  xml_write_map(mapFile, aMap);
  fclose(mapFile);
  
  
  printf("Wrote maps to disk\n");
}

map* load_origin_map() {
  return load_map_by_name(ORIGIN_MAP_NAME);
}

map* load_map_by_name(char* name) {
  //maybe do some common setup here or somethings
  map* a_map = load_map(name);
  return a_map;
}

map* make_origin_map() {
  map* origin_map = create_map(ORIGIN_MAP_NAME);
  
  int cols = 1;
  int rows = 1;
  int width = getScreenWidth() / cols;
  int height = getScreenHeight() / rows;
  spatial_hash_map* map = create_shm(width, height, cols, rows);

  plane* plane = create_plane(map, MAIN_PLANE_NAME);
  virt_pos center = (virt_pos){.x = getScreenWidth() / 2, .y = getScreenHeight() / 2};

  compound* user = makeCrab();
  //compound* user = tunctish();
  //compound* user = makeTrashCan();
  make_compound_user(user);
  add_compound_to_plane(plane, user);
  offset_compound(user, &center);

  insert_load_zone_into_plane(ORIGIN_MAP_NAME, BEACH_MAP_NAME, plane, MAIN_PLANE_NAME, &center, &ORIGIN_BEACH_POS);
  //insert_load_zone_into_plane(ORIGIN_MAP_NAME, BASIC_MAP_NAME, plane, MAIN_PLANE_NAME, &center, &ORIGIN_BEACH_POS);
  add_plane(origin_map, plane);
  return origin_map;
}

map* make_beach_map() {
  map* beach = create_map(BEACH_MAP_NAME);

  int map_width = getScreenWidth() * 3.5;
  int map_height = getScreenHeight() * 1.5;
  virt_pos cent = (virt_pos){.x = map_width * 0.5, .y = map_height * 0.5};
  room starting_room;
  init_room(&starting_room, &cent, map_width * 0.8, map_height * 0.8);
  int cols = 20;
  int rows = 14;
  int width = map_width / cols;
  int height = map_height / rows;

  spatial_hash_map* map = create_shm(width, height, cols, rows);

  plane* bg = create_plane(map, BACKGROUND_PLANE_NAME);
  plane* main = create_plane(map, MAIN_PLANE_NAME);
  plane* fg = create_plane(map, FOREGROUND_PLANE_NAME);

  virt_pos slime_offset = calc_room_offset(&starting_room, 0.5, -0.9);
  compound_spawner* slime_spawn = create_compound_spawner(BLUE_SLIME_SPAWN, -1 , slime_offset.x, slime_offset.y);
  add_spawner_to_plane(main, slime_spawn);

  virt_pos trashcan_offset = calc_room_offset(&starting_room, 0, -0.9);
  trashcan_offset.x = ORIGIN_BEACH_POS.x;
  compound_spawner* trashcan_spawn = create_compound_spawner(TRASHCAN_SPAWN, -1 , trashcan_offset.x, trashcan_offset.y);
  add_spawner_to_plane(main, trashcan_spawn);

  trashcan_offset.y -= 50;
  compound_spawner* gohei_spawn = create_compound_spawner(GOHEI_SPAWN, -1, trashcan_offset.x, trashcan_offset.x);
  add_spawner_to_plane(main, gohei_spawn);

  virt_pos monster_offset = calc_room_offset(&starting_room,-0.5, -0.9);
  compound_spawner* monster_spawn = create_compound_spawner(TEST_SPAWN, -1, monster_offset.x, monster_offset.y);
  add_spawner_to_plane(main, monster_spawn);
  
  gohei_spawn = create_compound_spawner(GOHEI_SPAWN, -1, monster_offset.x, monster_offset.y);
  add_spawner_to_plane(main, gohei_spawn);
  
  
  body* left_wall = add_wall_to_room(&starting_room, ROOM_WALL_LEFT, -1,1);
  body* right_wall = add_wall_to_room(&starting_room, ROOM_WALL_RIGHT, -1,1);
  body* down_wall = add_wall_to_room(&starting_room, ROOM_WALL_BOTTOM, -1, 1);
  tile_texture_for_body(down_wall, SAND_FN, 3,3,0,0);
  compound* wall_comp = create_compound();
  add_body_to_compound(wall_comp, left_wall);
  add_body_to_compound(wall_comp, right_wall);
  add_body_to_compound(wall_comp, down_wall);
  add_compound_to_plane(main, wall_comp);
  
  
  body* aTree = NULL;
  virt_pos tree_offset = *zero_pos;
  int tree_width = 48;
  int tree_height = 190;
  aTree = quick_block(tree_width,tree_height, BEACH_TREE_FN);
  tree_offset = calc_room_offset(&starting_room, -0.8, -1);
  tree_offset.y -= tree_height * 0.5;
  offset_body(aTree, &tree_offset);
  add_compound_to_plane(bg, mono_compound(aTree));

  aTree = quick_block(tree_width,tree_height, BEACH_TREE_FN);
  tree_offset = calc_room_offset(&starting_room, -0.6, -1);
  tree_offset.y -= tree_height * 0.5;
  offset_body(aTree, &tree_offset);
  add_compound_to_plane(bg, mono_compound(aTree));

  aTree = quick_block(tree_width,tree_height, BEACH_TREE_FN);
  tree_offset = calc_room_offset(&starting_room, -0.3, -1);
  tree_offset.y -= tree_height * 0.5;
  offset_body(aTree, &tree_offset);
  add_compound_to_plane(bg, mono_compound(aTree));

  aTree = quick_block(tree_width,tree_height, BEACH_TREE_FN);
  tree_offset = calc_room_offset(&starting_room, 0.2, -1);
  tree_offset.y -= tree_height * 0.5;
  offset_body(aTree, &tree_offset);
  add_compound_to_plane(bg, mono_compound(aTree));

  aTree = quick_block(tree_width,tree_height, BEACH_TREE_FN);
  tree_offset = calc_room_offset(&starting_room, 0.6, -1);
  tree_offset.y -= tree_height * 0.5;
  offset_body(aTree, &tree_offset);
  add_compound_to_plane(bg, mono_compound(aTree));
  
  body* long_plat = quick_block(600, 30 , EYE_FN);
  virt_pos long_plat_offset = calc_room_offset(&starting_room, 0.5, -0.5);
  offset_body(long_plat, &long_plat_offset);
  set_moi(get_fizzle(long_plat), 10);
  add_compound_to_plane(main, mono_compound(long_plat));

  add_plane(beach, bg);
  add_plane(beach, main);
  add_plane(beach, fg);
  return beach;
}

map* make_basic_map() {
  map* basic = create_map(BASIC_MAP_NAME);

  int map_width = getScreenWidth() * 1.5;
  int map_height = getScreenHeight() * 1.5;
  virt_pos cent = (virt_pos){.x = map_width * 0.5, .y = map_height * 0.5};
  room starting_room;
  init_room(&starting_room, &cent, map_width * 0.8, map_height * 0.8);
  int cols = 20;
  int rows = 14;
  int width = map_width / cols;
  int height = map_height / rows;

  spatial_hash_map* map = create_shm(width, height, cols, rows);


  plane* main = create_plane(map, MAIN_PLANE_NAME);

  
  body* left_wall = add_wall_to_room(&starting_room, ROOM_WALL_LEFT, -1,1);
  body* right_wall = add_wall_to_room(&starting_room, ROOM_WALL_RIGHT, -1,1);
  body* down_wall = add_wall_to_room(&starting_room, ROOM_WALL_BOTTOM, -1, 1);
  body* up_wall = add_wall_to_room(&starting_room, ROOM_WALL_TOP, -1, 1);
  
  compound* wall_comp = create_compound();
  add_body_to_compound(wall_comp, left_wall);
  add_body_to_compound(wall_comp, right_wall);
  add_body_to_compound(wall_comp, down_wall);
  add_body_to_compound(wall_comp, up_wall);
  add_compound_to_plane(main, wall_comp);
  
  
  
  add_plane(basic, main);
  return basic;
}
