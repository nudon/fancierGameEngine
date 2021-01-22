#include "objects.h"
#include "compound.h"
#include "room.h"
#include "spawner.h"
#include "guts.h"

//char* spawn_array[] = {TRASHCAN_SPAWN, BLUE_SLIME_SPAWN, GOHEI_SPAWN, CEILING_GRASS_SHORT, FLOOR_GRASS_SHORT, /* more things here */ NULL };

#define MAIN_SET_SIZE 20
spawner_set* main_set = NULL;

void init_spawn_set() {
  if (main_set != NULL) {
    fprintf(stderr, "error, re-initializing spawn set\n");
  }
  main_set = create_spawner_set(MAIN_SET_SIZE);
  spawner_set_append(main_set, TEST_SPAWN, &monkey);
  spawner_set_append(main_set, TRASHCAN_SPAWN, &makeTrashCan);
  spawner_set_append(main_set, GOHEI_SPAWN, &makeGohei);
  spawner_set_append(main_set, CEILING_GRASS_SHORT, &ceiling_grass_short);
  spawner_set_append(main_set, FLOOR_GRASS_SHORT, &floor_grass_short);
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

compound* roper(int segments) {
  compound* centComp = create_compound();
  add_smarts_to_comp(centComp);
  body* b;
  virt_pos* center = &(virt_pos){.x = 0, .y = 0};
  for (int i = 0; i < segments; i++) {
    b = makeNormalBody(4, 2);
    set_moi(get_fizzle(b), INFINITY); 
    set_body_center(b, center);
    add_body_to_compound(centComp, b);
  }
  tether_join_compound(centComp, rev_one_way_tether);
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
  shared_input_set_origin(*torso_si, torso, SI_CENTER);
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
  shared_input_set_origin(*torso_si, torso, SI_CENTER);
  
  set_shared_input(torso, torso_si);
  add_body_to_compound(comp, torso);

  eyes(torso);
  hand(torso);
  
  set_compound_gravity(comp, g);

  set_hunter(get_comp_attributes(get_compound_smarts(comp)), 1);
  set_prey(get_comp_attributes(get_compound_smarts(comp)), 1);
  return comp;
}

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
  bp = createRectangle(bl, width);
  lp = createRectangle(sl, width);
  rp = createRectangle(sl, width);
  set_rotation(lp, side_theta);
  set_rotation(rp, side_omega);
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
  shared_input_set_origin(*si, bottom, SI_CENTER);
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
  set_body_center(handle, center);
  virt_pos offset = (virt_pos){.x = handle_width * 3 / 2, -handle_height / 2};
  right_side = makeRectangleBody(side_width,side_width);
  left_side = makeRectangleBody(side_width,side_width);
  set_body_center(right_side, &offset);
  offset.x *= -1;
  set_body_center(left_side, &offset);
  shared_input_set_origin(*si, handle, SI_CENTER);
  set_shared_input(handle, si);
  set_shared_input(right_side, si);
  set_shared_input(left_side, si);
  add_body_to_compound(gohei,handle);
  add_body_to_compound(gohei,right_side);
  add_body_to_compound(gohei,left_side);
  ceiling_roper(4, right_side);
  ceiling_roper(4, left_side);
  //sub_roper(4, right_side, one_way_tether);
  //sub_roper(4, left_side, one_way_tether);

  //set_compound_gravity(gohei, g);
  
  smarts* sm = get_compound_smarts(gohei);
  att* c_atts = get_comp_attributes(sm);
  set_holdable(c_atts, 1);

  return gohei;
}

compound* ceiling_grass_short() {
  compound* c = create_compound();
  body* root = makeNormalBody(3, 2);
  add_body_to_compound(c, root);
  ceiling_roper(5, root);
  return c;
}

compound* floor_grass_short() {
  compound* c = create_compound();
  body* root = makeNormalBody(3, 2);
  add_body_to_compound(c, root);
  floor_roper(5, root);
  return c;
}
