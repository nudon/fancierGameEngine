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




static int init(SDL_Renderer** ret);
static void myClose();
static int loadMedia();

void setBGColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void update_tethers(gen_list* tetherList);
void update_plane(plane* plane);
void update_compounds(spatial_hash_map* map, gen_list* compound_list);
void main_test(camera* cam, map* map);

//Globals
static SDL_Window* gWin = NULL;

static int FPS_CAP = 60;


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
  collider* coll = NULL;

  fizzle* fizz = NULL;
  vector_2 input = *zero_vec;
  virt_pos trans_disp = *zero_pos;
  double rot_disp = 0;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    body_curr = get_bodies(aCompound)->start;
    while (body_curr != NULL) { 
      aBody = (body*)body_curr->stored;
      fizz = aBody->fizz;
      coll = aBody->coll;
      
      trans_disp = *zero_pos;
      input = *zero_vec;
      rot_disp = 0;
      apply_poltergeist(aBody->polt, aBody, &input, &rot_disp);
      add_velocity(fizz, &input);
      add_rotational_velocity(fizz, rot_disp);
      fizzle_update(fizz);
      update_pos_with_curr_vel(&trans_disp, fizz);
      update_rot_with_current_vel(&rot_disp, fizz);
      if (update(map, coll, &trans_disp, rot_disp)) {
	//marks that body actually moved, and needs to be collision checked
	aBody->status = 1;
      }
      
      body_curr = body_curr->next;
    }
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
    comp_curr = comp_curr->next;
  }
}

void update_plane(plane* plane) {
  update_tethers(get_tethers(plane));
  check_events(get_shm(plane), get_events(plane));
  update_compounds(get_shm(plane), get_compounds(plane));
}

void draw_plane(plane* plane, camera* cam) {
  gen_node* curr_compound;
  compound* temp;
  SDL_SetRenderDrawColor(cam->rend,0,0,0,0xff);
  curr_compound = get_compounds(plane)->start;
  while (curr_compound != NULL) {
    temp = (compound*)curr_compound->stored;
    draw_compound_outline(cam, temp);
    //draw_compound_picture(cam, temp);
    curr_compound = curr_compound->next;
  }
}


void main_loop() {
  double ms_per_frame = 1000.0 / FPS_CAP;
  double time;
  int update_wait;      
  int quit = 0;
  time_update();
  camera* cam = getCam();
  map* map = getMap();
  while (!quit) {
    SDL_SetRenderDrawColor(cam->rend,0xff,0xff,0xff,0xff);
    SDL_RenderClear(cam->rend);
    set_dT(time_since_update());
    time_update();
    
    main_test(cam, map);
    
    update_wait = ms_per_frame - get_dT();	
    if (update_wait > 0) {
      SDL_Delay(update_wait);
    }
    draw_events_in_map(getCam(), map);
    draw_load_zones_in_map(getCam(), map);
    draw_hash_map(getCam(), get_shm((plane*)get_planes(map)->start->stored)); 
    SDL_RenderPresent(cam->rend);
    time = time_since_update();
    update_wait = ms_per_frame - time;
    if (update_wait > 0) {
      SDL_Delay(update_wait);
    }
    quit = getQuit();
    if (map != getMap()) {
      //map changed, save current one
    }
    map = getMap();
  }
}

void main_test(camera* cam, map* map) {
  gen_node* curr = get_planes(map)->start;
  plane* p = NULL;
  while(curr != NULL) {
    p = (plane*)curr->stored;
    update_plane(p);
    draw_plane(p, cam);
    curr = curr->next;
  }
  check_load_triggers(map);  
}

int main(int argc, char** args) {
  SDL_Renderer* rend = NULL;
  pixel_pos corner = (pixel_pos){.x = 0, .y = 0};
  if (init(&rend) == 0) {
    if (loadMedia() == 0) {
      camera mainCam;
      mainCam.rend = rend;
      mainCam.dest = NULL;
      mainCam.corner = &corner;
      setCam(&mainCam);

      write_maps_to_disk();
      
      //map* map = make_origin_map();
      //map* map = make_street_map();

      map* map = load_origin_map();
      map_load_create_travel_lists(map);
      setMap(map);
      main_loop(map);
      
    }
    else {
      fprintf(stderr,"LoadMediaFailed\n");
    }    
  }
  else {
    fprintf(stderr, "Init failed\n");    
  }
  myClose();
  return 0;
}



int init(SDL_Renderer** ret_rend) {
  ///ret_rend = NULL;
  int fail = 0;
  if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
    gWin = SDL_CreateWindow("SDL, Now with renderererers",
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    getScreenWidth(),
			    getScreenHeight(),
			    SDL_WINDOW_SHOWN);
    if (gWin != NULL) {
      *ret_rend = SDL_CreateRenderer(gWin, -1, SDL_RENDERER_ACCELERATED);
      if (ret_rend != NULL) {
	SDL_SetRenderDrawColor(*ret_rend, 0xff, 0xff, 0xff, 0xff);
	int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
	if ((IMG_Init(imgFlags) & imgFlags)) {
	  if (TTF_Init() != -1) {
	    //initialize my libraries
	    init_poltergeists();
	    init_events();
	    init_map_load();	    
	  }
	  else {
	    fail = 5;
	    fprintf(stderr, "Error in loading TTF libraries: %s\n",
		    TTF_GetError());
	  }
	}
	else {
	  fail = 4;
	  fprintf(stderr, "Error in loading img loading librarys: %s \n",
		  IMG_GetError());
	}
      }
      else {
	fail = 3;
	fprintf(stderr, "Error in creating renderer for gWin: %s \n",
		SDL_GetError());
      }      
    }
    else {
      fail = 2;
      fprintf(stderr, "Window Could not be created: %s\n",
	      SDL_GetError());
    }
  }
  else {
    fprintf(stderr, "Error in init video %s\n",
	    SDL_GetError() );
    fail =1;
  }
  return fail;
}

int loadMedia() {
  return 0;
}

void myClose() {
  SDL_DestroyWindow(gWin);
  gWin = NULL;
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}
