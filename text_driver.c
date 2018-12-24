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



static int init(SDL_Renderer** ret);

static void myClose();

static int loadMedia();

//static int startDebug();

//static void gameLoop();

void setBGColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);




//maybe
void body_move(spatial_hash_map* map, body* body, virt_pos* td, double rd) {
  //potentially move fizzle_update/resolve collisions to another thing
  //under motivation of moving all objects, then dealing with colliisons
  collider* coll = body->coll;
  fizzle* fizz = body->fizz;
  int t_collide = 0, r_collide = 0;
  vector_2 loc = *zero_vec;
  if (td->x != 0 || td->y != 0) {
    virt_pos_to_vector_2(td, &loc);
    add_velocity(fizz, &loc);
    fizzle_update(fizz);
    update_pos_with_curr_vel(td, fizz);
    t_collide = safe_move(map, coll, td);
  }
  if (rd != 0) {
    r_collide = safe_rotate(map, coll, rd);
  }
  if (t_collide != 0 || r_collide != 0) {
    resolve_collisions(map, body);
  }
  
}

void update_bodies(spatial_hash_map* map, gen_list* l) {
  gen_node* curr = l->start;
  collider* coll;
  body* aBody;
  fizzle* fizz;
  virt_pos trans_disp;
  double rot_disp;
  vector_2 loc;  
  while (curr != NULL) { 
    aBody = (body*)curr->stored;
    fizz = aBody->fizz;
    coll = aBody->coll;
    
    trans_disp = *zero_pos;
    rot_disp = 0;
    apply_poltergeist(aBody->polt, map, aBody, &trans_disp, &rot_disp);
    virt_pos_to_vector_2(&trans_disp, &loc);
    add_velocity(fizz, &loc);
    fizzle_update(fizz);
    update_pos_with_curr_vel(&trans_disp, fizz);
    //fprintf(stderr, "tranDisp is %d, %d\n", trans_disp.x, trans_disp.y);
    //aBody->status = 0;
    if (update(map, coll, &trans_disp, rot_disp)) {
      aBody->status = 1;
    }
    
    curr = curr->next;
  }
  curr = l->start;
  //curr = NULL;
  while(curr != NULL) {
    aBody = (body*)curr->stored;
    coll = aBody->coll;
    //fprintf(stderr, "just checking!\n");
    if (aBody->status != 0) {
      aBody->status = 0;
      //fprintf(stderr, "actually checking for collisions !\n");
      if (anyCollisions(map, coll)) {
	//fprintf(stderr, "oh no theres a collisionsasdfasf!\n");
	resolve_collisions(map, aBody);
	//fizz->velocity = *zero_vec;
      }
    }
  curr = curr->next;
  }
}

//Globals
static SDL_Window* gWin = NULL;

static int FPS_CAP = 60;


void print_out_extreme_diffs(polygon* poly) {
  double min, max;
  vector_2 norm;
  virt_pos center = poly->center;
  virt_pos* centa = &center;
  //centa = NULL;
  for(int i = 0; i < poly->sides; i++) {
    get_actual_normal(poly, i, &norm);
    extreme_projections_of_polygon(poly, centa, &norm, &min, &max);
    printf("{ min: %f, max: %f diff: %f }",min, max, fabs(max - min));
  }
  printf("\n");
}



int main_test(camera* cam, gen_list* list, spatial_hash_map* map) {
  //fprintf(stderr, "executing main test\n");
  int quit = 0;
  gen_node* curr;
  body* temp;
  SDL_SetRenderDrawColor(cam->rend,0xff,0xff,0xff,0xff);
  //SDL_SetRenderDrawColor(cam->rend,0,0,0,0xff);
  SDL_RenderClear(cam->rend);
  //read input
  //move things
  //draw screen
 
  quit = get_quit();
  update_bodies( map, list);

  //collider_list_node* refs = coll->collider_node;
  //update hash map for coll
  //remove_collider_from_shm_entries(map, refs, refs->active_cells);

  //entries_for_collider(map, coll, refs->active_cells);
  //add_collider_to_shm_entries(map, refs, refs->active_cells);

  //draw_hash_map(cam, map);
  SDL_SetRenderDrawColor(cam->rend,0,0,0,0xff);
  curr = list->start;
  while (curr != NULL) {
    temp = curr->stored;
    draw_polygon_outline(cam, temp->coll->shape);

    
    curr = curr->next;
  }

  
  //  SDL_Delay(updateWait);
  //SDL_RenderPresent(cam->rend);
  return quit;
}
  

int main(int argc, char** args) {
  //  quit = 0;
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

      int width = getScreenWidth() / cols;
      int height = getScreenHeight() / rows;
      spatial_hash_map* map = create_shm(width, height, cols, rows);
      gen_list* ttd = createGen_list();
      gen_list* collbbtd = createGen_list();
      double ms_per_frame = 1000.0 / FPS_CAP;
      int update_wait;
 
      makeUserBody(ttd);
      makeTriangle(ttd);

      gen_node* curr = ttd->start;
      
      while(curr != NULL) {
	collider* coll = ((body*)curr->stored)->coll;
	insert_collider_in_shm(map, coll);
	curr = curr->next;
      }
      double time;
      time_update();
      while (!quit) {
	set_dT(time_between_updates());
	time_update();
	
	main_test(&mainCam, ttd, map);
       	gen_node* curr = collbbtd->start;
	while (curr != NULL) {
	  draw_bbox(&mainCam, (collider*)(curr->stored));
	  curr = curr->next;
	}

	//draw_hash_map(&mainCam, map); 
	SDL_RenderPresent(mainCam.rend);
	quit = get_quit();
	
	time = time_between_updates();
	update_wait = ms_per_frame - time;
	printf("waiting time is %d\n", update_wait);
	if (update_wait > 0) {
	  SDL_Delay(update_wait);
	}
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
	    //setActiveMenu(NULL);
	    //setMainMenu(createMainMenu());
	    //setMapEditMenu(createMapEditMainMenu());
	    //setGameState(GAMERUN);
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


void makeWalls(gen_list* list) {
  polygon* walls[4];
  for(int i = 0; i < 4; i++) {
    walls[i] = createPolygon(4);
    make_square(walls[i]);
    if (i % 2 == 0) {
      stretch_deform_vert(walls[i], getScreenHeight() / 2);
      stretch_deform_horz(walls[i], getScreenWidth() / 8);
      walls[i]->center.y = getScreenHeight() / 2;
      if (i == 2) {
	walls[i]->center.x = 0;
      }
      else {
	walls[i]->center.x = getScreenWidth();
      }
    }
    else {
      stretch_deform_horz(walls[i], getScreenWidth() / 2);
      stretch_deform_vert(walls[i], getScreenHeight() / 8);
      walls[i]->center.x = getScreenWidth() / 2;
      if (i == 3) {
	walls[i]->center.y = 0;
      }
      else {
	walls[i]->center.y = getScreenHeight();
      }
	  
    }
    generate_normals_for_polygon(walls[i]);
  }
  collider* wallColliders [4];
  body* body;
  fizzle* fizz;
  for (int i = 0; i < 4; i++) {
    wallColliders[i] = make_collider_from_polygon(walls[i]);
    fizz = createFizzle();
    init_fizzle(fizz);
    double wallMass = 9000;
    fizz->mass = wallMass;
    body = createBody(fizz, wallColliders[i]);
    prependToGen_list(list, createGen_node(body));
    //prependToGen_list(collbbtd, createGen_node(wallColliders[i]));
  }
}

void makeTriangle(gen_list* list) {
  polygon* triom = NULL;
  collider* bos = NULL;
  body* body;
  fizzle* fizz;
  triom = createPolygon(3);
  triom->center.x = getScreenWidth() / 2;
  triom->center.y = getScreenHeight() / 2;
  make_normal_polygon(triom);
  generate_normals_for_polygon(triom);
  triom->scale = 3;
  bos = make_collider_from_polygon(triom);
  fizz = createFizzle();
  init_fizzle(fizz);
  body = createBody(fizz, bos);
  prependToGen_list(list, createGen_node(body));
}

void makeUserBody(gen_list* list) {
  polygon* mainPoly = createNormalPolygon(4);
  collider* coll;
  fizzle* fizz;
  body* body;
  poltergeist* polt;
  mainPoly->center.x = getScreenWidth() / 4;
  mainPoly->center.y = getScreenHeight() / 2;
  mainPoly->scale = 4;
  coll = make_collider_from_polygon(mainPoly);
  fizz = createFizzle();
  init_fizzle(fizz);
  //set_gravity(fizz, &((vector_2){.v1 = 0, .v2 = .1}));
  body = createBody(fizz, coll);
  polt = make_poltergeist();
  give_user_poltergeist(polt);
  body->polt = polt;
  prependToGen_list(list, createGen_node(body));
}
