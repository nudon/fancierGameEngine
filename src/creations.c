//sort of behaves like a collection of prefabs

#include <stdlib.h>
#include <stdio.h>

#include "creations.h"
#include "graphics.h"
#include "gi.h"
#include "map_io.h"
#include "sizes.h"
#include "names.h"
#include "room.h"
#include "objects.h"
#include "game_state.h"

virt_pos ORIGIN_BEACH_POS = (virt_pos){.x= 345, .y=270};


event* make_load_event(virt_pos* cent) {
  polygon* poly = createNormalPolygon(7);
  set_center(poly, cent);
  event* load = make_event(poly);
  return load;
}


void insert_load_zone_into_plane(char* from_map, char* to_map, plane* from_plane, char* to_plane_name, virt_pos* from_pos, virt_pos* to_pos) {
  char* from_plane_name = get_plane_name(from_plane);
  event* load_event = make_load_event(from_pos);
  load_zone* lz = make_load_zone(from_map, to_map, from_plane_name, to_plane_name, to_pos, load_event);
  add_load_zone_to_plane(from_plane, lz);
}

void make_compound_user(compound* comp) {
  body* head = get_compound_head(comp);
  poltergeist* polt = make_poltergeist();
  smarts * sm = get_compound_smarts(comp);
  att* comp_att = get_comp_attributes(sm);
  give_user_poltergeist(polt);
  set_poltergeist(head, polt);
  set_user(comp_att, 1);
  set_travel(comp_att, 1);
  setUser(comp);
}

void make_compound_builder(compound* comp) {
  body* head = get_compound_head(comp);
  poltergeist* polt = make_poltergeist();
  //smarts * sm = get_compound_smarts(comp);
  //att* comp_att = get_comp_attributes(sm);
  give_builder_poltergeist(polt);
  set_poltergeist(head, polt);
  //set_user(comp_att, 1);
  //set_travel(comp_att, 1);
  setBuilder(comp);
}

map* load_origin_map() {
  return load_map(ORIGIN_MAP_NAME);
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
  //compound* user = makeGohei();
  //compound* user = makeTrashCan();
  //compound* user = roper(9);
  make_compound_user(user);
  add_compound_to_plane(plane, user);
  offset_compound(user, &center);

  //insert_load_zone_into_plane(ORIGIN_MAP_NAME, BEACH_MAP_NAME, plane, MAIN_PLANE_NAME, &center, &ORIGIN_BEACH_POS);
  //insert_load_zone_into_plane(ORIGIN_MAP_NAME, BASIC_MAP_NAME, plane, MAIN_PLANE_NAME, &center, &ORIGIN_BEACH_POS);
  insert_load_zone_into_plane(ORIGIN_MAP_NAME, SUBWAY_CAR_MAP_NAME, plane, MAIN_PLANE_NAME, &center, &ORIGIN_BEACH_POS);
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

map* make_subway_car_map() {
  map* basic = create_map(SUBWAY_CAR_MAP_NAME);
  virt_pos spawn_point = *zero_pos;
  compound_spawner* spawner = NULL;
  int map_width = getScreenWidth() * 3.5;
  int map_height = getScreenHeight() * 1.5;
  virt_pos cent = (virt_pos){.x = map_width * 0.5, .y = map_height * 0.5};
  room car;
  init_room(&car, &cent, map_width * 0.8, map_height * 0.8);
  int cols = 20;
  int rows = 14;
  int width = map_width / cols;
  int height = map_height / rows;
  spatial_hash_map* map = create_shm(width, height, cols, rows);


  plane* main = create_plane(map, MAIN_PLANE_NAME);

  
  body* left_wall = add_wall_to_room(&car, ROOM_WALL_LEFT, -1,1);
  body* right_wall = add_wall_to_room(&car, ROOM_WALL_RIGHT, -1,1);
  body* down_wall = add_wall_to_room(&car, ROOM_WALL_BOTTOM, -1, 1);
  body* up_wall = add_wall_to_room(&car, ROOM_WALL_TOP, -1, 1);
  
  compound* wall_comp = create_compound();
  add_body_to_compound(wall_comp, left_wall);
  add_body_to_compound(wall_comp, right_wall);
  add_body_to_compound(wall_comp, down_wall);
  add_body_to_compound(wall_comp, up_wall);

  //wont work because of fizzles being lost in serialization, will have to make a spawner
  //ceiling_roper(2, up_wall);
  spawn_point = calc_room_offset(&car, 0, 0.9);
  spawner = create_compound_spawner(CEILING_GRASS_SHORT, -1 , spawn_point.x, spawn_point.y);
  //add_spawner_to_plane(main, spawner);

  spawn_point = calc_room_offset(&car, -0.5, -0.9);
  spawner = create_compound_spawner(FLOOR_GRASS_SHORT, -1 , spawn_point.x, spawn_point.y);
  //add_spawner_to_plane(main, spawner);
  
  add_compound_to_plane(main, wall_comp);
  add_plane(basic, main);
  return basic;
}

void write_maps_to_disk() {
  map* aMap = NULL;
 
  aMap = make_beach_map();
  save_map(aMap, BEACH_MAP_NAME);

  aMap = make_basic_map();
  save_map(aMap, BASIC_MAP_NAME);
    
  aMap = make_subway_car_map();
  save_map(aMap, SUBWAY_CAR_MAP_NAME);
  
  printf("Wrote maps to disk\n");
}
