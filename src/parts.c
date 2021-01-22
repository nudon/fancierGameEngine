#include "parts.h"
#include "shapes.h"
#include "guts.h"

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
  add_body_to_compound(comp, right_eye_anchor);
  
  add_body_to_compound(comp, left_eye);
  add_body_to_compound(comp, right_eye);

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

void sub_roper(int seg, body* base, tether* param)  {
  compound* comp = get_owner(base);
  body* b1, *b2;
  polygon* p1, *p2;
  tether* body_teth;
  virt_pos center = get_body_center(base);
  b1 = base;
  for (int i = 0; i < seg; i++) {
    b2 = makeNormalBody(4, 2);
    set_moi(get_fizzle(b2), INFINITY);
    set_body_center(b2, &center);

   
    if (b2 != NULL) {
      p1 = get_polygon(get_collider(b1));
      p2 = get_polygon(get_collider(b2));
      body_teth = create_tether_blank(read_only_polygon_center(p1),read_only_polygon_center(p2),get_fizzle(b1),get_fizzle(b2));
      copy_tether_params(param, body_teth);
      
      add_tether_to_compound(comp, body_teth);
    }
    add_body_to_compound(comp, b2);
    b1 = b2;
  }
}

void var_grav_roper(int segments, body* base, vector_2* grav) {
  gen_node* curr = get_bodies(get_owner(base))->end;
  body* b = NULL;
  sub_roper(segments, base, one_way_tether);
  curr = curr->next;
  while(curr != NULL) {
    b = (body*)curr->stored;
    set_body_gravity(b, grav);
    curr = curr->next;
  }
}




void ceiling_roper(int segments, body* base) {
  vector_2 more_g = *g;
  more_g.v2 *= 16;
  var_grav_roper(segments, base, &more_g);
}

void floor_roper(int segments, body* base) {
  vector_2 more_g = *g;
  more_g.v2 *= -16;
  var_grav_roper(segments, base, &more_g);
}
