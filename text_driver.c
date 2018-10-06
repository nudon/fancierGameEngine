#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "collider.h"
#include "myVector.h"
#include "graphics.h"
#include "body.h"
#include "game_state.h"

//hash map seems to generally work. generally errors are in determining max_occupied_cells, otherwise it's good
//rarely crashes sometimes upon collisions too, usually when a thing is just barely colliding, from what I recall
//thinking about keeping track of another list. would keep track of things that are currently colliding in shm
//not a whole lot of benefit, asside from being able to defer collision detection all at once after everything moves. instead of noticing and resolving collisions as updtating positions, I'd be noticing and adding collision info to some list.

//also, for movement, need to modify get input for polygon to respond to held down keys. seems to ignore them
//for movement, want to continue at speed
//eventually, when I get to jumping, want to add some variable amount to upwards velocity.
//probably some 0-1 scale of (get_dT * gravity) * up_direction
//so at first, little negative acceleration, but then eventually it kicks in
//can probably have the 0-1 scale be related to time_spent_holding_jump and max_jumping_period
//if tphj < mjp, scale = (1 - tshj / mjp)
//else scale = 0;



//so, next up is a physics system. had an idea for objects in equilibrium states, such as objects resting on ground, in which case no calculations are done constantly reverifying their equilibirum. instead when touched by other objects, they get tested for if they are still in equilibrium
//so far, collusiong detection is just stopping object. sat is unwieldy for determining normal of collision,


//beyond that, had an idea for a poltergeist object which is responsible for moving objectes around.
//the main playable character would just be a poltergeist with a function that takes keyboard input

//also, was thinking about stuff like parallax scrolling and z-levels. given some z-level and some horizon line, should be able to work out some geomety to get scale to shrink/grow obkect
//untested solution,but pretty easy. write virt_positions relative to vanishing point, then scale down.

//had a simple idea for an editor. would have a list of polygons, have left/rightarrow + some mod move along list
//some key and prompt to create a new normal polycon
//maybe some arrow + mod to stretch_deform the normal polygon.
//some other things for rotation. 
//prototypes 
static int init(SDL_Renderer** ret);

static void myClose();

static int loadMedia();

//static int startDebug();

//static void gameLoop();

void setBGColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

//maybe
void body_move(spatial_hash_map* map, body* body, virt_pos* t_disp, double r_disp) {
  //potentially move fizzle_update/resolve collisions to another thing
  //under motivation of moving all objects, then dealing with colliisons
  collider* coll = body->coll;
  polygon* poly = coll->shape;
  fizzle* fizz = body->fizz;
  int t_collide = 0, r_collider = 0;
  vector_2 loc = *zero_pos;
  if (td->x != 0 || td->y != 0) {
    virt_pos_to_vector(d_disp, &loc);
    add_velocity(fizz, &loc);
    fizzle_update(fizz);
    update_pos_with_curr_vel(&td, fizz);
    t_collide = safe_move(map, coll, &td);
  }
  if (r_disp != 0) {
    r_collide = safe_rotate(map, coll, rd);
  }
  if (t_collide != 0 || r_collide != 0) {
    resolve_collisions(map, body);
  }
  
}

//Globals
static SDL_Window* gWin = NULL;

static int FPS_CAP = 60;

//static int updateWait = 15;


int get_input_for_polygon(polygon* poly, virt_pos* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  int mov_delta = 1;
  double rot_delta = .3;
  double scale_delta = 1;
  //virt_pos trans_disp = (virt_pos){.x = 0, .y = 0};
  //double rot_disp = 0;
  while (SDL_PollEvent(&e) != 0 ) {
    if (e.type == SDL_QUIT) {
      quit = 1;
    }
    else if (e.type == SDL_KEYDOWN) {
      if (!(SDL_GetModState() & KMOD_CTRL)) {
	switch(e.key.keysym.sym) {
	case SDLK_UP:
	  trans_disp->y -= mov_delta;
	  break;
	case SDLK_DOWN:
	  trans_disp->y += mov_delta;
	  break;
	case SDLK_RIGHT:
	  trans_disp->x += mov_delta;
	  break;
	case SDLK_LEFT:
	  trans_disp->x -= mov_delta;
	  break;
	default:

	  break;
	    
	}
      }
      else {
	switch(e.key.keysym.sym) {
	  //scaling is bad thing to do with the shm being a thing
	  //will probably crash when increasing size too much and generating entries for things
	case SDLK_UP:
	  poly->scale += scale_delta;
	  break;
	case SDLK_DOWN:
	  poly->scale -= scale_delta;
	  break;
	case SDLK_RIGHT:
	  *rot_disp += rot_delta;
	  break;

	case SDLK_LEFT:
	  *rot_disp -= rot_delta;
	  break;
	default:

	  break;
	}
      }
    }
  }
  set_quit(quit);
  return quit;
}

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
  int quit = 0;
  gen_node* curr;
  body* temp;
  static polygon* controlll = NULL;
  static collider* coll = NULL;
  static body* body = NULL;

  if (controlll == NULL) {
    controlll = createPolygon(4);
    controlll->center.x = SCREEN_WIDTH / 4;
    controlll->center.y = SCREEN_HEIGHT / 2;
    make_normal_polygon(controlll);
    generate_normals_for_polygon(controlll);
    controlll->scale = 4;
    //stretch_deform_vert(controlll, 2);
    coll = make_collider_from_polygon(controlll);
    insert_collider_in_shm(map, coll);
    fizzle* fizz = createFizzle();
    init_fizzle(fizz);
    body = createBody(fizz, coll);
    prependToGen_list(list, createGen_node(body));
  }
  SDL_SetRenderDrawColor(cam->rend,0xff,0xff,0xff,0xff);
  //SDL_SetRenderDrawColor(cam->rend,0,0,0,0xff);
  SDL_RenderClear(cam->rend);
  //read input
  //move things
  //draw screen
  virt_pos td = (virt_pos){.x = 0, .y = 0};
  double rd = 0;
  int safe;
  get_input_for_polygon(controlll, &td, &rd);
  quit = get_quit();
  if (td.x != 0 || td.y != 0) {
    //add input to body
    vector_2 loc;
    fizzle* fizz = body->fizz;
    virt_pos_to_vector_2(&td, &loc);
    //add_acceleration(fizz, &loc);
    add_velocity(fizz, &loc);
    fizzle_update(fizz);
    //fprintf(stderr, "Velocity is %f\n", vector_2_magnitude(&(fizz->velocity)));
    update_pos_with_curr_vel(&td, fizz);
    safe = safe_move(map, coll, &td);
    if (safe == 0) {
      resolve_collisions(map, body);
      //fizz->velocity = *zero_vec;
    }
      }
  if (rd != 0) {
    safe_rotate(map, coll, rd);
  }
  //collider_list_node* refs = coll->collider_node;
  //update hash map for coll
  //remove_collider_from_shm_entries(map, refs, refs->active_cells);

  //entries_for_collider(map, coll, refs->active_cells);
  //add_collider_to_shm_entries(map, refs, refs->active_cells);
  draw_bbox(cam, coll);
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
      //do stuff
      //INITIALIZE hash map
      //create colliders for walls and moveable polygon
      //insert

      //fore walls, use the make squares function specifically
      //then use the streatch deforem vertical and horz to make wall shape
      //

      int cols = 20;
      int rows = 14;
      //cols = 4;
      //rows = 4;
      int width = SCREEN_WIDTH / cols;
      int height = SCREEN_HEIGHT / rows;
      spatial_hash_map* map = create_shm(width, height, cols, rows);
      gen_list* ttd = createGen_list();
      gen_list* collbbtd = createGen_list();
      polygon* walls[4];
      double ms_per_frame = 1000 / FPS_CAP;
      int update_wait;
      for(int i = 0; i < 4; i++) {
	walls[i] = createPolygon(4);
	make_square(walls[i]);
	if (i % 2 == 0) {
	  stretch_deform_vert(walls[i], SCREEN_HEIGHT / 2);
	  stretch_deform_horz(walls[i], SCREEN_WIDTH / 8);
	  walls[i]->center.y = SCREEN_HEIGHT / 2;
	  if (i == 2) {
	    walls[i]->center.x = 0;
	  }
	  else {
	    walls[i]->center.x = SCREEN_WIDTH;
	  }
	}
	else {
	  
	  stretch_deform_horz(walls[i], SCREEN_WIDTH / 2);
	  stretch_deform_vert(walls[i], SCREEN_HEIGHT / 8);
	  walls[i]->center.x = SCREEN_WIDTH / 2;
	  if (i == 3) {
	    walls[i]->center.y = 0;
	  }
	  else {
	    walls[i]->center.y = SCREEN_HEIGHT;
	  }
	  
	}
	generate_normals_for_polygon(walls[i]);
      }
      collider* wallColliders [4];
      body* body;
      fizzle* fizz;
      for (int i = 0; i < 4; i++) {
	wallColliders[i] = make_collider_from_polygon(walls[i]);
	insert_collider_in_shm(map, wallColliders[i]);
	fizz = createFizzle();
	init_fizzle(fizz);
	body = createBody(fizz, wallColliders[i]);
	prependToGen_list(ttd, createGen_node(body));
	prependToGen_list(collbbtd, createGen_node(wallColliders[i]));
      }
      polygon* triom = NULL;
      collider* bos = NULL;
      triom = createPolygon(3);
      triom->center.x = SCREEN_WIDTH / 2;
      triom->center.y = SCREEN_HEIGHT / 2;
      make_normal_polygon(triom);
      generate_normals_for_polygon(triom);
      triom->scale = 3;
      bos = make_collider_from_polygon(triom);
      fizz = createFizzle();
      init_fizzle(fizz);
      body = createBody(fizz, bos);
      prependToGen_list(ttd, createGen_node(body));
      insert_collider_in_shm(map, bos);
      prependToGen_list(collbbtd, createGen_node(bos));
      while (!quit) {
	time_update();
	main_test(&mainCam, ttd, map);
       	gen_node* curr = collbbtd->start;
	while (curr != NULL) {
	  draw_bbox(&mainCam, (collider*)(curr->stored));
	  curr = curr->next;
	}
	set_dT(time_between_updates());
	update_wait = ms_per_frame - get_dT();
	if (update_wait > 0) {
	  SDL_Delay(update_wait);
	}
	SDL_RenderPresent(mainCam.rend);
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
			    SCREEN_WIDTH,
			    SCREEN_HEIGHT,
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


