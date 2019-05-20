#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "collider.h"
#include "myVector.h"
#include "graphics.h"
#include "body.h"
#include "game_state.h"
#include "input.h"
#include "parallax.h"
#include "events.h"
#include "plane.h"

#include "map_io.h"

polygon* make_event_poly(polygon* shape);

static int init(SDL_Renderer** ret);
static void myClose();
static int loadMedia();

void setBGColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

compound* makeWalls();
compound* makeTriangle();
compound* makeUserBody();
compound* makeCentipede(int seg);


void update_tethers(gen_list* tetherList);
void update_plane(plane* plane);
void update_bodies(spatial_hash_map* map, gen_list* compound_list);
void main_test(camera* cam, plane* plane);

//Globals
//Globals
static SDL_Window* gWin = NULL;

static int FPS_CAP = 60;



gen_list* global_tethers = NULL;

void update_tethers(gen_list* tetherList) {
  tether* teth;
  gen_node* curr = tetherList->start;
  while(curr != NULL) {
    teth = (tether*)curr->stored;
    apply_tether(teth);
    curr = curr->next;
  }
}

void update_plane(plane* plane) {
  update_bodies(plane->map, plane->compounds_in_plane);
}

void update_bodies(spatial_hash_map* map, gen_list* compound_list) {
  //gen_list* compound_list;
  gen_node* comp_curr = compound_list->start;
  gen_node* body_curr;
  compound* aCompound;
  body* aBody;
  collider* coll;

  fizzle* fizz;
  vector_2 input;
  virt_pos trans_disp;
  double rot_disp;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    body_curr = aCompound->bp->start;
    while (body_curr != NULL) { 
      aBody = (body*)body_curr->stored;
      fizz = aBody->fizz;
      coll = aBody->coll;
    
      trans_disp = *zero_pos;
      input = *zero_vec;
      rot_disp = 0;
      apply_poltergeist(aBody->polt, map, aBody, &input, &rot_disp);
      add_velocity(fizz, &input);
      fizzle_update(fizz);
      update_pos_with_curr_vel(&trans_disp, fizz);
      if (update(map, coll, &trans_disp, rot_disp)) {
	aBody->status = 1;
      }
    
      body_curr = body_curr->next;
    }
    comp_curr = comp_curr->next;
  }
  comp_curr = compound_list->start;
  while(comp_curr != NULL){
    aCompound = (compound*)comp_curr->stored;
    body_curr = aCompound->bp->start;
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


void main_test(camera* cam, plane* plane) {
  gen_node* curr_compound;
  compound* temp;
  SDL_SetRenderDrawColor(cam->rend,0xff,0xff,0xff,0xff);
  SDL_RenderClear(cam->rend);
  update_tethers(global_tethers);
  update_plane(plane);
  //update_bodies( map, list);
  SDL_SetRenderDrawColor(cam->rend,0,0,0,0xff);
  curr_compound = plane->compounds_in_plane->start;
  while (curr_compound != NULL) {
    temp = (compound*)curr_compound->stored;
    draw_compound_outline(cam, temp);
    curr_compound = curr_compound->next;
  }
}

int main(int argc, char** args) {
  SDL_Renderer* rend = NULL;
  pixel_pos corner = (pixel_pos){.x = 0, .y = 0};
  int quit = 0;
  if (init(&rend) == 0) {
    if (loadMedia() == 0) {
      camera mainCam;
      mainCam.rend = rend;
      mainCam.dest = NULL;
      mainCam.corner = &corner;
      setCam(&mainCam);

      int cols = 20;
      int rows = 14;
      //cols = 10;
      //rows = 7;
      //cols = 1;
      //rows = 1;

      int width = getScreenWidth() / cols;
      int height = getScreenHeight() / rows;
      spatial_hash_map* map = create_shm(width, height, cols, rows);

      global_tethers = createGen_list();
      //gen_list* compound_list = createGen_list();
      plane* plane = create_plane(map);
      
 
      //compound* user = makeUserBody();
      compound* user = makeCentipede(3);
      compound* triangle = makeTriangle();
      compound* walls = makeWalls();
      add_compound_to_plane(plane, user);
      add_compound_to_plane(plane, triangle);
      add_compound_to_plane(plane, walls);

      //test_plane_parse(plane);
      //exit(44);
      
      double ms_per_frame = 1000.0 / FPS_CAP;
      int update_wait;      
      double time;
      time_update();
      while (!quit) {
	set_dT(time_since_update());
	time_update();
	//check_events(map, eventList);
	main_test(&mainCam, plane);
	update_wait = ms_per_frame - get_dT();	
	if (update_wait > 0) {
	  SDL_Delay(update_wait);
	}
	draw_hash_map(&mainCam, map); 
	SDL_RenderPresent(mainCam.rend);
	
	time = time_since_update();
	update_wait = ms_per_frame - time;
	if (update_wait > 0) {
	  SDL_Delay(update_wait);
	}
	quit = get_quit();
      }
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
	    //successfull init
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
  poltergeist* polt;
  mainPoly->center->x = getScreenWidth() / 4;
  mainPoly->center->y = getScreenHeight() / 2;
  mainPoly->scale = 10;
  coll = make_collider_from_polygon(mainPoly);
  fizz = createFizzle();
  init_fizzle(fizz);
  //set_gravity(fizz, &((vector_2){.v1 = 0, .v2 = .1}));
  body = createBody(fizz, coll);
  polt = make_poltergeist();
  give_user_poltergeist(polt);
  body->polt = polt;
  //prependToGen_list(list, createGen_node(body));
  add_body_to_compound(userComp, body);
  return userComp;
}

compound* makeCentipede(int segments) {
  compound* centComp = create_compound();
  polygon* poly;
  collider* coll;
  fizzle* fizz;
  body* body;
  poltergeist* polt;
  for (int i = 0; i < segments; i++) {
    poly = createNormalPolygon(8);
    poly->center->x = getScreenWidth() / 4;
    poly->center->y = getScreenHeight() / 2;
    poly->scale = 2;
    coll = make_collider_from_polygon(poly);
    fizz = createFizzle();
    init_fizzle(fizz);
    body = createBody(fizz, coll);
    if (i == 0) {
      polt = make_poltergeist();
      give_user_poltergeist(polt);
      body->polt = polt;
    }
    add_body_to_compound(centComp, body);
  }
  tether_join_compound(centComp, NULL, global_tethers);
  return centComp;
}


polygon* make_event_poly(polygon* shape) {
  polygon* new = createNormalPolygon(12);

  return new;
}
