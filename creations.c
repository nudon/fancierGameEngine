//sort of behaves like a collection of prefabs

#include <stdlib.h>
#include <stdio.h>

#include "creations.h"
#include "graphics.h"
#include "gi.h"
#include "map_io.h"



/// picture/media definitions

#define MEDIA_FOLDER "./media/"

#define CRAB_PIC_FN MEDIA_FOLDER"crab.png"

/// helper functions


//handles initial setup
body* blankBody(polygon* base) {
  body* b = NULL;
  collider* c = make_collider_from_polygon(base);
  fizzle* f = createFizzle();
  init_fizzle(f);
  b = createBody(f, c);
  return b;
}

body* makeNormalBody(int sides) {
  polygon* ngon = NULL;
  body* b;
  ngon = createNormalPolygon(sides);
  b = blankBody(ngon);
  return b;
}

//functions for automatically creating/spacing multiple objects
#define HORZ_CHAIN 0
#define VERT_CHAIN 1
 
compound* makeBlockChain(virt_pos* start_pos, int len, int chain_type) {
  vector_2 dir;
  double disp = 0;
  int width = 30;
  int height = 30;
  if (chain_type == HORZ_CHAIN) {
    dir = (vector_2){.v1 = 1, .v2 = 0};
    disp = width;
  }
  else if (chain_type == VERT_CHAIN) {
    dir = (vector_2){.v1 = 0, .v2 = 1};
    disp = height;
  }
  else {
    fprintf(stderr, "unset chain dir\n");
  }
  body* base_block = makeBlock(width, height);
  return makeBodyChain(base_block, start_pos, len, &dir, disp);
}
 
// duplicates a body in the specified way and returns compound, original body is not used
//also makes compound uniform, meaning bodies rotate and move in sync
compound* makeBodyChain(body* start, virt_pos* start_pos,  int len, vector_2* dir, double disp) { 
  compound* chain = create_compound();
  make_compound_uniform(chain);
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
    virt_pos_add(p->center, &disp_pos, p->center);
    add_body_to_compound(chain, temp);
  }
  return chain;
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
    aWall->center->x = xVal;
    aWall->center->y = yVal;
    generate_normals_for_polygon(aWall);

    b = blankBody(aWall);
    set_mass(getFizzle(b), wallMass);
    add_body_to_compound(wallCompound, b);
  }
  return wallCompound;
}

//blocks are just rectangles are meant to be used as floor/wall tiles
body* makeBlock (int width, int height) {
  body* b = NULL;
  polygon* p = createRectangle(width, height);
  b = blankBody(p);
  fizzle* f = getFizzle(b);
  set_mass(f, INFINITY);
  set_bounce(f, 0.1);
  return b;
}

compound* makeCentipede(int segments, gen_list* tethers, virt_pos* center) {
  compound* centComp = create_compound();
  body* body;
  for (int i = 0; i < segments; i++) {
    body = makeNormalBody(8);
    *(getCenter(body)) = *center;
    add_body_to_compound(centComp, body);
  }
  tether_join_compound(centComp, NULL, tethers);
  return centComp;
}

compound* makeCrab(virt_pos* center) {
  compound* centComp = create_compound();
  polygon* poly;
  collider* coll;
  fizzle* fizz;
  body* body;
  poly = createRectangle(60, 40);

  *(poly->center) = *center;
  coll = make_collider_from_polygon(poly);
  fizz = createFizzle();
  init_fizzle(fizz);
  set_bounce(fizz, 0.3);
  vector_2 g = (vector_2){.v1 = 0, .v2 = 500};
  set_gravity(fizz, &g);
  body = createBody(fizz, coll);
  add_body_to_compound(centComp, body);

 
  set_picture_by_name(body, CRAB_PIC_FN );
  return centComp;
}

//goal is to make a trashcan like this
/*
  |  | 
  |__|
 */
compound* makeTrashCan(virt_pos* center) {
  compound* can = create_compound();
  make_compound_uniform(can);
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
  add_body_to_compound(can, bottom);
  add_body_to_compound(can, lside);
  add_body_to_compound(can, rside);
  return can;
}

/// special things

//compound takes user input
void make_compound_user(compound* comp) {
  body* head = (body*)get_bodies(comp)->start->stored;
  poltergeist* polt = make_poltergeist();
  give_user_poltergeist(polt);
  head->polt = polt;
  set_user(get_attributes(comp), 1);
  set_travel(get_attributes(comp), 1);
}

//just some standard map transitions, 
polygon* make_event_poly(polygon* shape) {
  polygon* new = createNormalPolygon(12);

  return new;
}

char* ORIGIN_MAP_NAME = "origin.map";

char* ROOM_MAP_NAME = "room.map";

char* STREET_MAP_NAME = "streets.map";

char* BACKGROUND_PLANE_NAME = "background";

char* MAIN_PLANE_NAME = "main";

virt_pos ORIGIN_ROOM_POS = (virt_pos){.x= 345, .y=270};
virt_pos ROOM_STREET_POS = (virt_pos){.x= 120, .y=270};
virt_pos STREET_ROOM_POS = (virt_pos){.x= 570, .y=70};

void write_maps_to_disk() {
  map* origin = make_origin_map();
  FILE* origin_map = fopen(ORIGIN_MAP_NAME, "w+");
  xml_write_map(origin_map, origin);
  fclose(origin_map);
  
  map* street = make_street_map();
  FILE* streets_map = fopen(STREET_MAP_NAME, "w+");
  xml_write_map(streets_map, street);
  fclose(streets_map);
  
  map* room = make_room_map();
  FILE* room_map = fopen(ROOM_MAP_NAME, "w+");
  xml_write_map(room_map, room);
  fclose(room_map);
  
  printf("Wrote maps to disk\n");
}

map* load_origin_map() {
  return load_map_by_name(ORIGIN_MAP_NAME);
}

map* load_room_map() {
  return load_map_by_name(ROOM_MAP_NAME);
}

map* load_streets_map() {
  return load_map_by_name(STREET_MAP_NAME);
}

map* load_map_by_name(char* name) {
  //maybe do some common setup here or somethings
  map* a_map = load_map(name);
  return a_map;
}

map* make_origin_map() {
  map* origin_map = create_map(ORIGIN_MAP_NAME);
  
  int cols = 4;
  int rows = 4;
  int width = getScreenWidth() / cols;
  int height = getScreenHeight() / rows;
  spatial_hash_map* map = create_shm(width, height, cols, rows);

  plane* plane = create_plane(map, MAIN_PLANE_NAME);
  virt_pos center = (virt_pos){.x = getScreenWidth() / 2, .y = getScreenHeight() / 2};
  compound* user = makeCrab(&center);
  compound* walls = makeWalls();
  make_compound_user(user);
  set_hunter(get_attributes(user), 1);
  add_compound_to_plane(plane, user);
  add_compound_to_plane(plane, walls);
  
  
  event* load_event = make_load_event(&ORIGIN_ROOM_POS);
  load_zone* lz = make_load_zone(ORIGIN_MAP_NAME, ROOM_MAP_NAME, MAIN_PLANE_NAME, MAIN_PLANE_NAME, &ORIGIN_ROOM_POS, load_event);
  add_load_zone_to_plane(plane, lz);
  add_plane(origin_map, plane);
  return origin_map;
}

map* make_room_map() {
  map* origin_map = create_map(ROOM_MAP_NAME);

  int map_width = 1600;
  int map_height = 800;
  int cols = 20;
  int rows = 14;
  int width = map_width / cols;
  int height = map_height / rows;

  double floor_height = map_height * 2 / 3;
  spatial_hash_map* map = create_shm(width, height, cols, rows);

  plane* plane = create_plane(map, MAIN_PLANE_NAME);
  
  virt_pos pos = (virt_pos){.x = map_width * 2 / 3, .y = map_height / 2}; 
  compound* triangle = create_compound();
  body* temp = makeNormalBody(3);
  poltergeist* p = make_poltergeist();
  give_standard_poltergeist(p);
  temp->polt = p;
  *(getCenter(temp)) = pos;
  add_body_to_compound(triangle, temp);
  set_prey(get_attributes(triangle), 1);
  add_compound_to_plane(plane, triangle);

  
  virt_pos can_pos = (virt_pos){.x = ORIGIN_ROOM_POS.x, floor_height - 15}; // -15 is half of the floor block width
  compound* can = makeTrashCan(&can_pos);
  add_compound_to_plane(plane, can);
    

  virt_pos start = (virt_pos){.x = 0, .y = floor_height};
  compound* floor = makeBlockChain(&start, 20, HORZ_CHAIN);
  add_compound_to_plane(plane, floor);

  
  make_basic_vision_event((body*)get_bodies(triangle)->start->stored);
  
  event* load_event = make_load_event(&ROOM_STREET_POS);
  load_zone* lz = make_load_zone(ROOM_MAP_NAME, STREET_MAP_NAME, MAIN_PLANE_NAME, MAIN_PLANE_NAME, &ROOM_STREET_POS, load_event);
  add_load_zone_to_plane(plane, lz);
  add_plane(origin_map, plane);
  return origin_map;
}

map* make_street_map() {
  map* street_map = create_map(STREET_MAP_NAME);
  
  int cols = 20;
  int rows = 7;
  int width = 2 * getScreenWidth() / cols;
  int height = getScreenHeight() / rows;
  spatial_hash_map* map = create_shm(width, height, cols, rows);

  plane* plane = create_plane(map, MAIN_PLANE_NAME);
  //compound* triangle = makeTriangle();
  //set_prey(get_attributes(triangle), 1);
  //add_compound_to_plane(plane, triangle);
  
  //struct event_struct* vi = make_basic_vision_event((body*)get_bodies(triangle)->start->stored);
  //add_event_to_plane(plane, vi);

  event* load_event = make_load_event(&STREET_ROOM_POS);
  load_zone* lz = make_load_zone(STREET_MAP_NAME, ROOM_MAP_NAME, MAIN_PLANE_NAME, MAIN_PLANE_NAME, &STREET_ROOM_POS, load_event);
  add_load_zone_to_plane(plane, lz);
  
  add_plane(street_map, plane);
  return street_map;
}

event* make_load_event(virt_pos* cent) {
  polygon* poly = createNormalPolygon(7);
  set_center(poly, cent);
  event* load = make_event(poly);
  return load;
}
