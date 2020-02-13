#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "graphics.h"
#include "game_state.h"
#include "input.h"
#include "events.h"
#include "shapes.h"
#include "creations.h"
#include "map_io.h"
#include "map.h"
#include "builder.h"

static void myClose();


void main_loop();
void update_map(map* map);
void update_plane(plane* plane);
void update_tethers(gen_list* tetherList);
void update_compounds(spatial_hash_map* map, gen_list* compound_list);
int move_body(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp);
void move_compound(spatial_hash_map* map, compound* c);
//Globals

int main(int argc, char** args) {
  init_graphics();
  
  write_maps_to_disk();
  
  map* map = make_origin_map();
  //map* map = make_beach_map();
  
  map_load_create_travel_lists(map);
  setMap(map);
  main_loop(map);
  
  myClose();
  return 0;
}


void play_logic(map* m) {
  update_map(m);
  inc_physics_frame();
}




void main_loop() {
  double ms_per_frame = 1000.0 / FPS;
  int update_wait;      
  camera* cam = getCam();
  map* map = getMap();
  int mode = 0;
  update_corner(cam);
  time_update();
  while (!getQuit()) {
    SDL_SetRenderDrawColor(cam->rend,0xff,0xff,0xff,SDL_ALPHA_OPAQUE);
    SDL_RenderClear(cam->rend);
    set_dT(time_since_update());
    time_update();
    
    mode = getMode();
    if (mode == PLAY_MODE) {
      play_logic(map);
    }
    else if (mode == BUILD_MODE) {
      builder_logic(map);
    }
    
    update_corner(cam);
    draw_map(cam, map);
    update_wait = ms_per_frame - time_since_update();	
    if (update_wait > 0) {
      SDL_Delay(update_wait);
    }
    
    SDL_RenderPresent(cam->rend);
    
    if (map != getMap()) {
      //map changed, save current one
    }
    map = getMap();
  }
}

void update_map(map* map) {
  gen_node* curr = get_planes(map)->start;
  plane* p = NULL;
  while(curr != NULL) {
    p = (plane*)curr->stored;
    update_plane(p);
    curr = curr->next;
  }
  check_load_triggers(map);
}

void update_plane(plane* plane) {
  update_tethers(get_tethers(plane));
  check_events(get_shm(plane), get_events(plane));
  update_compounds(get_shm(plane), get_compounds(plane));
}

void update_tethers(gen_list* tetherList) {
  tether* teth;
  gen_node* curr = tetherList->start;
  while(curr != NULL) {
    teth = (tether*)curr->stored;
    apply_tether(teth);
    curr = curr->next;
  }
}

void update_compounds(spatial_hash_map* map, gen_list* compound_list) {
  //gen_list* compound_list;
  gen_node* comp_curr = compound_list->start;
  gen_node* body_curr;
  compound* aCompound;
  body* aBody = NULL;

  fizzle* fizz = NULL;
  virt_pos trans_disp = *zero_pos;
  double rot_disp = 0;
  //first traversal, do poltergeist/events/general smarts stuff
  comp_curr = compound_list->start;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    body_curr = get_bodies(aCompound)->start;
    
    update_tethers(get_compound_tethers(aCompound));
    
    while (body_curr != NULL) { 
      aBody = (body*)body_curr->stored;
      
      check_events(map, get_body_events(aBody));
      update_smarts(get_body_smarts(aBody));
      run_body_poltergeist(aBody);
      
      body_curr = body_curr->next;
    }
    update_smarts(get_compound_smarts(aCompound));
    
    comp_curr = comp_curr->next;
  }
  //move all bodies 
  comp_curr = compound_list->start;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    body_curr = get_bodies(aCompound)->start;
    
      
    while (body_curr != NULL) { 
      aBody = (body*)body_curr->stored;
      fizz = get_fizzle(aBody);
      trans_disp = *zero_pos;
      rot_disp = 0.0;
      
      calc_change(fizz, &trans_disp, &rot_disp);
      if (move_body(map, aBody, &trans_disp, rot_disp)) {
	set_move_status(aBody, 1);
      }
      
      body_curr = body_curr->next;
    }
    comp_curr = comp_curr->next;
  }
  //move all compounds
  comp_curr = compound_list->start;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
      
    move_compound(map, aCompound);
    
    comp_curr = comp_curr->next;
  }
  comp_curr = compound_list->start;
  //body collisions
  while(comp_curr != NULL) {
    aCompound = (compound*)comp_curr->stored;
    body_curr = get_bodies(aCompound)->start;
    while(body_curr != NULL) {
      aBody = (body*)body_curr->stored;
      if (get_move_status(aBody) != 0) {
	set_move_status(aBody, 0);
	resolve_collisions(map, aBody);
      }
      body_curr = body_curr->next;
    }
    comp_curr = comp_curr->next;
  }
  comp_curr = compound_list->start;
  //compound compounds
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    move_compound(map, aCompound);
    comp_curr = comp_curr->next;
 
  }
}

int move_body(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp) {
  shared_input* si = get_shared_input(b);
  if (si != NULL) {
    add_to_shared_input(t_disp, r_disp, si);
    return 1;
  }
  else {
    calc_change(get_fizzle(b), t_disp, &r_disp);
    return update(map, get_collider(b), t_disp, r_disp);
  }
}

void move_compound(spatial_hash_map* map, compound* c) {
  gen_node* n  = NULL;
  body* b = NULL;
  virt_pos rot_offset = *zero_pos;
  virt_pos curr_offset = *zero_pos;
  virt_pos orig_offset = *zero_pos;
  virt_pos t_disp = *zero_pos;
  polygon* head = get_polygon(get_collider(get_compound_head(c)));
  double r_disp = 0;
  virt_pos avg_t_disp = *zero_pos;
  double avg_r_disp = 0;
  shared_input* si = NULL;
  n = get_bodies(c)->end;
  while(n != NULL) {
    b = (body*)n->stored;
    si = get_shared_input(b);
    rot_offset = get_rotation_offset(get_polygon(get_collider(b)));
    if (si != NULL)  {
      get_avg_movement(si, &avg_t_disp, &avg_r_disp);
      orig_offset = *zero_pos;
      curr_offset = get_si_offset(b);
      t_disp = *zero_pos;
      pull_shared_reflections(b);
      if (!isZeroPos(&rot_offset)) {
	//modify t_disp for poly so it looks like it wasn't rotated about it's center
	virt_pos_rotate(&rot_offset, get_rotation(head), &orig_offset);
	virt_pos_sub(&orig_offset, &curr_offset, &t_disp);
	virt_pos_sub(&avg_t_disp,&t_disp, &t_disp);
      }
      else {
	//no rotation offset, nothing special happens
	t_disp = avg_t_disp;
      }
      r_disp = avg_r_disp;
      update(map, get_collider(b), &t_disp, r_disp);
    }
    n = n->prev;
  }
}


void myClose() {
  quit_graphics();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}
