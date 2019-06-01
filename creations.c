//sort of behaves like a collection of prefabs

#include <stdlib.h>
#include <stdio.h>

#include "creations.h"
#include "graphics.h"
#include "gi.h"
#include "map_io.h"

compound* makeWalls() {
  compound* wallCompound = create_compound();
  polygon* walls[4];
  int xVal, yVal;
  for(int i = 0; i < 4; i++) {
    walls[i] = createPolygon(4);
    make_square(walls[i]);
    if (i % 2 == 0) {
      stretch_deform_vert(walls[i], getScreenHeight() / 2);
      stretch_deform_horz(walls[i], getScreenWidth() / 8);
      yVal = getScreenHeight() / 2;
      if (i == 2) {
	xVal = 0;
      }
      else {
        xVal = getScreenWidth();
      }
    }
    else {
      stretch_deform_horz(walls[i], getScreenWidth() / 2);
      stretch_deform_vert(walls[i], getScreenHeight() / 8);
      xVal = getScreenWidth() / 2;
      if (i == 3) {
	yVal = 0;
      }
      else {
        yVal = getScreenHeight();
      }
    }
    walls[i]->center->x = xVal;
    walls[i]->center->y = yVal;
    generate_normals_for_polygon(walls[i]);
  }
  collider* wallColliders [4];
  body* body;
  fizzle* fizz;
  double wallMass = 9000;
  wallMass = INFINITY;
  for (int i = 0; i < 4; i++) {
    wallColliders[i] = make_collider_from_polygon(walls[i]);
    fizz = createFizzle();
    init_fizzle(fizz);

    fizz->mass = wallMass;
    body = createBody(fizz, wallColliders[i]);
    add_body_to_compound(wallCompound, body);
    //prependToGen_list(list, createGen_node(body));
    //prependToGen_list(collbbtd, createGen_node(wallColliders[i]));
  }
  return wallCompound;
}

compound* makeTriangle() {
  compound* triangleComp = create_compound();
  polygon* tri = NULL;
  collider* coll = NULL;
  body* body;
  fizzle* fizz;
  tri = createNormalPolygon(3);
  tri->center->x = getScreenWidth() / 2;
  tri->center->y = getScreenHeight() / 2;
  tri->scale = 3;
  coll = make_collider_from_polygon(tri);
  fizz = createFizzle();
  init_fizzle(fizz);
  body = createBody(fizz, coll);
  poltergeist* polt = make_poltergeist();
  give_standard_poltergeist(polt);
  body->polt = polt;
  //prependToGen_list(list, createGen_node(body));
  add_body_to_compound(triangleComp, body);
  return triangleComp;
}

compound* makeUserBody() {
  compound* userComp = create_compound();
  polygon* mainPoly = createNormalPolygon(4);
  collider* coll;
  fizzle* fizz;
  body* body;
  mainPoly->center->x = getScreenWidth() / 4;
  mainPoly->center->y = getScreenHeight() / 2;
  mainPoly->scale = 10;
  coll = make_collider_from_polygon(mainPoly);
  fizz = createFizzle();
  init_fizzle(fizz);
  body = createBody(fizz, coll);
  add_body_to_compound(userComp, body);
  make_compound_user(userComp);
  return userComp;
}

compound* makeCentipede(int segments, gen_list* tethers) {
  compound* centComp = create_compound();
  polygon* poly;
  collider* coll;
  fizzle* fizz;
  body* body;
  for (int i = 0; i < segments; i++) {
    poly = createNormalPolygon(8);
    poly->center->x = getScreenWidth() / 4;
    poly->center->y = getScreenHeight() / 2;
    poly->scale = 2;
    coll = make_collider_from_polygon(poly);
    fizz = createFizzle();
    init_fizzle(fizz);
    body = createBody(fizz, coll);
    add_body_to_compound(centComp, body);
  }
  tether_join_compound(centComp, NULL, tethers);
  return centComp;
}

void make_compound_user(compound* comp) {
  body* head = (body*)get_bodies(comp)->start->stored;
  poltergeist* polt = make_poltergeist();
  give_user_poltergeist(polt);
  head->polt = polt;
  set_user(get_attributes(comp), 1);
  set_travel(get_attributes(comp), 1);
}


polygon* make_event_poly(polygon* shape) {
  polygon* new = createNormalPolygon(12);

  return new;
}

char* ORIGIN_MAP_NAME = "origin.map";

char* STREET_MAP_NAME = "streets.map";

char* BACKGROUND_PLANE_NAME = "background";

char* MAIN_PLANE_NAME = "main";

virt_pos ORIGIN_STREET_POS = (virt_pos){.x= 120, .y=270};
virt_pos STREET_ORIGIN_POS = (virt_pos){.x= 270, .y=50};

void write_maps_to_disk() {
  map* origin = make_origin_map();
  FILE* origin_map = fopen(ORIGIN_MAP_NAME, "w+");
  xml_write_map(origin_map, origin);
  fclose(origin_map);
  map* street = make_street_map();
  FILE* streets_map = fopen(STREET_MAP_NAME, "w+");
  xml_write_map(streets_map, street);
  fclose(streets_map);

  printf("Wrote maps to disk\n");
}

map* load_origin_map() {
  return load_map_by_name(ORIGIN_MAP_NAME);
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
  
  int cols = 20;
  int rows = 14;
  int width = getScreenWidth() / cols;
  int height = getScreenHeight() / rows;
  spatial_hash_map* map = create_shm(width, height, cols, rows);

  plane* plane = create_plane(map, MAIN_PLANE_NAME);
  //compound* user = makeUserBody();
  compound* user = makeCentipede(1, get_tethers(plane));
  make_compound_user(user);
  compound* triangle = makeTriangle();
  compound* walls = makeWalls();
  set_hunter(get_attributes(user), 1);
  set_prey(get_attributes(triangle), 1);
  add_compound_to_plane(plane, user);
  //add_compound_to_plane(plane, triangle);
  //add_compound_to_plane(plane, walls);

  gen_list* eventList = get_events(plane);
  struct event_struct* vi = make_basic_vision_event((body*)get_bodies(triangle)->start->stored);
  gen_node* node = createGen_node(vi);
  prependToGen_list(eventList,node );

  event* load_event = make_load_event(&ORIGIN_STREET_POS);
  load_zone* lz = make_load_zone(ORIGIN_MAP_NAME, STREET_MAP_NAME, MAIN_PLANE_NAME, MAIN_PLANE_NAME, &STREET_ORIGIN_POS, load_event);
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
  //compound* user = makeUserBody();
  //compound* user = makeCentipede(1, get_tethers(plane));
  compound* triangle = makeTriangle();
  //compound* walls = makeWalls();
  //set_hunter(get_attributes(user), 1);
  set_prey(get_attributes(triangle), 1);
  //add_compound_to_plane(plane, user);
  add_compound_to_plane(plane, triangle);
  //add_compound_to_plane(plane, walls);

  struct event_struct* vi = make_basic_vision_event((body*)get_bodies(triangle)->start->stored);
  add_event_to_plane(plane, vi);

  //event* load_event = make_load_event(&ORIGIN_STREET_POS);
  //load_zone* lz = make_load_zone(ORIGIN_MAP_NAME, STREET_MAP_NAME, MAIN_PLANE_NAME, MAIN_PLANE_NAME, &STREET_ORIGIN_POS, load_event);
  //add_load_zone_to_plane(plane, lz);
  
  add_plane(street_map, plane);
  return street_map;
}

event* make_load_event(virt_pos* cent) {
  polygon* poly = createNormalPolygon(7);
  set_center(poly, cent);
  event* load = make_event(poly);
  return load;
}
