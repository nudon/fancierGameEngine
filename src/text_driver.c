#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "graphics.h"
#include "game_state.h"
#include "input.h"
#include "events.h"
#include "creations.h"
#include "map_io.h"
#include "map.h"

static void myClose();


void main_loop();
void update_map(map* map);
void update_plane(plane* plane);
void update_tethers(gen_list* tetherList);
void update_compounds(spatial_hash_map* map, gen_list* compound_list);

//Globals

static int FPS_CAP = 60;

int main(int argc, char** args) {
  init_graphics();
  
  write_maps_to_disk();
  
  map* map = load_origin_map();
  //map* map = make_beach_map();
  
  map_load_create_travel_lists(map);
  setMap(map);
  main_loop(map);
  
  myClose();
  return 0;
}


void main_loop() {
  double ms_per_frame = 1000.0 / FPS_CAP;
  int update_wait;      
  camera* cam = getCam();
  map* map = getMap();
  update_corner(cam);
  time_update();
  while (!getQuit()) {
    SDL_SetRenderDrawColor(cam->rend,0xff,0xff,0xff,SDL_ALPHA_OPAQUE);
    SDL_RenderClear(cam->rend);
    set_dT(time_since_update());
    time_update();

    update_map(map);
    update_corner(cam);
    //printf("loop tick\n");
    draw_map(cam, map);
    
    update_wait = ms_per_frame - get_dT_in_ms();	
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
  vector_2 input = *zero_vec;
  virt_pos trans_disp = *zero_pos;
  double rot_disp = 0;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    update_tethers(get_compound_tethers(aCompound));
    calc_new_dir(get_gi(aCompound));
    body_curr = get_bodies(aCompound)->start;
    while (body_curr != NULL) { 
      aBody = (body*)body_curr->stored;
      fizz = aBody->fizz;
      
      check_events(map, get_body_events(aBody));
      
      trans_disp = *zero_pos;
      input = *zero_vec;
      rot_disp = 0.0;
      apply_poltergeist(aBody->polt, aBody, &input, &rot_disp);
      add_velocity(fizz, &input);
      add_rotational_velocity(fizz, rot_disp);
      fizzle_update(fizz);
      update_pos_with_curr_vel(&trans_disp, fizz);
      update_rot_with_current_vel(&rot_disp, fizz);

      if (body_update(map, aBody, &trans_disp, rot_disp)) {
	aBody->status = 1;
      }
      body_curr = body_curr->next;
    }
    compound_update(map, aCompound);
    comp_curr = comp_curr->next;
  }
  comp_curr = compound_list->start;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    body_curr = get_bodies(aCompound)->start;
    while(body_curr != NULL) {
      aBody = (body*)body_curr->stored;
      if (aBody->status != 0) {
	aBody->status = 0;
	resolve_collisions(map, aBody);
      }
      body_curr = body_curr->next;
    }
    compound_update(map, aCompound);
    comp_curr = comp_curr->next;
  }
}

void myClose() {
  quit_graphics();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}
