#include "builder.h"
#include "shapes.h"
#include "objects.h"
#include "game_state.h"

int builder_spawn_flag;

int builder_plane_change_flag;

char* builder_spawn_name;

char** spawn_set = NULL;

compound_spawner* spawner_copy = NULL;
compound* spawned_item_copy = NULL;
map* spawner_map;

compound* builder = NULL;

void replace_spawned_copy();

//used to change spawned object
int spawner_index;
int spawner_len;
void inc_spawner_index() {
  if (spawn_set == NULL) {
    return;
  }
  spawner_index++;
  if (spawner_index == spawner_len) {
    spawner_index = 0;
  }
  replace_spawned_copy();
}
void dec_spawner_index() {
  if (spawn_set == NULL) {
    return;
  }
  spawner_index--;
  if (spawner_index == -1) {
    spawner_index = spawner_len - 1;
  }
  replace_spawned_copy();
}

char* spawner_entry() {
  return spawn_set[spawner_index];
}

void set_spawner_set(char* set[]) {
  if (set == NULL) {
    return;
  }
  int len = 0;
  while(set[len] != NULL) {
    len++;
  }
  spawner_index = 0;
  spawn_set = set;
  spawner_len = len;
  replace_spawned_copy();
}
void replace_spawned_copy() {
  plane* main_plane = (plane*)get_planes(spawner_map)->start->stored;
  virt_pos cent = get_body_center(get_compound_head(builder));
  if (spawner_copy != NULL) {
    free_compound_spawner(spawner_copy);
  }
  if (spawned_item_copy != NULL) {
    remove_compound_from_plane(main_plane, spawned_item_copy);
    free_compound(spawned_item_copy);
  }
  spawner_copy = create_compound_spawner(spawner_entry(), -1, 0,0);
  spawned_item_copy = compound_from_spawner(spawner_copy);
  add_compound_to_plane(main_plane, spawned_item_copy);

  set_compound_position(spawned_item_copy, &cent);
  
}


//used to change the active plane
gen_list* plane_list;
gen_node* plane_nd;
void next_plane() {
  plane_nd = plane_nd->next;
  if (plane_nd == NULL) {
    plane_nd = plane_list->start;
  }
  
}
void prev_plane() {
  plane_nd = plane_nd->prev;
  if (plane_nd == NULL) {
    plane_nd = plane_list->end;
  }
}
plane* plane_entry() {
  return (plane*)plane_nd->stored;
}
void set_plane_list(gen_list* list) {
  plane_list = list;
  plane_nd = plane_list->start;
}

void builder_logic(map* m) {
  static map* prev_m = NULL;
  if (builder == NULL) {
    spawner_map = create_map("builder_map");
    add_plane(spawner_map, create_plane(create_shm(100,100,1,1) , "main_plane"));
    
    builder = mono_compound(makeNormalBody(13, 9));
    make_compound_builder(builder);
    set_spawner_set(spawn_array);
  }
  if (prev_m != m) {
    set_plane_list(get_planes(m));
    prev_m = m;
  }
  run_body_poltergeist(get_compound_head(builder));
  virt_pos cent = get_body_center(get_compound_head(builder));
  if (builder_spawn_flag) {
    compound_spawner* spawner = create_compound_spawner(spawner_entry(), -1, cent.x, cent.y);
    plane* pl = plane_entry();
    add_spawner_to_plane(pl, spawner);
    trigger_spawner(spawner, pl);
    builder_spawn_flag = 0;
  }
  body* head = get_compound_head(spawned_item_copy);
  set_body_center(head, &cent);
  //set_compound_position(spawned_item_copy, &cent);
  play_logic(spawner_map);
  //draw_compound(getCam(), builder);
  draw_compound(getCam(), spawned_item_copy);
}

int builder_input(body* b, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  int up = 0, down = 0, left = 0, right = 0;
  int spawner = 0, spawn_idx_inc = 0, spawn_idx_dec = 0, plane_next = 0, plane_prev = 0;
  int mov_mag = 10;
  virt_pos offset = *zero_pos, t_vp = *zero_pos;
  while (SDL_PollEvent(&e) != 0 ) {
    if (e.type == SDL_QUIT) {
      quit = 1;
    }
    else if (e.type == SDL_KEYDOWN) {
      if ((SDL_GetModState() & KMOD_CTRL)) {
	mov_mag = 0;
	switch(e.key.keysym.sym) {
	case SDLK_RIGHT:
	  spawn_idx_inc = 1;
	  break;
	case SDLK_LEFT:
	  spawn_idx_dec = 1;
	  break;
	case SDLK_UP:
	  plane_prev = 1;
	  break;
	case SDLK_DOWN:
	  plane_next = 1;
	  break;
	case SDLK_m:
	  setMode(PLAY_MODE);
	  break;
	default:

	  break;
	}
      }
      else {
	switch(e.key.keysym.sym) {
	case SDLK_UP:
	  up = 1;
     	  break;
	case SDLK_DOWN:
	  down = 1;
	  break;
	case SDLK_RIGHT:
	  right = 1;
	  break;
	case SDLK_LEFT:
	  left = 1;
	  break;
	case SDLK_z:
	  spawner = 1;
	  break;
	case SDLK_x:

	  break;
	default:
	  break;
	}
      }
    }
  }
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_UP]) {
    up = 1;
  }
  if (state[SDL_SCANCODE_RIGHT]) {
    right = 1;
  }
  if (state[SDL_SCANCODE_DOWN]) {
    down = 1;
  }
  if (state[SDL_SCANCODE_LEFT]) {
    left = 1;
    }
  
  
  if (up) {
    offset.y -= mov_mag;
  }
  if (down) {
    offset.y += mov_mag;
  }
  if (left) {
    offset.x -= mov_mag;
  }
  if (right) {
    offset.x += mov_mag;
  }
  if (spawner) {
    builder_spawn_flag = 1;
  }
  if (spawn_idx_inc) {
    inc_spawner_index();
  }
  if (spawn_idx_dec) {
    dec_spawner_index();
  }
  if (plane_next) {
    next_plane();
  }
  if (plane_prev) {
    prev_plane();
  }

  t_vp = get_body_center(b);
  virt_pos_add(&t_vp, &offset, &t_vp);
  set_body_center(b, &t_vp);
  
  
  setQuit(quit);
  return quit;
}
