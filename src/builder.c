#include "builder.h"
#include "shapes.h"
#include "objects.h"
#include "names.h"
#include "game_state.h"
#include "map_io.h"
#include "spawner.h"
#include "guts.h"

void replace_spawned_copy();
void set_select_visuals(compound* c);
void unset_select_visuals(compound* c);


int builder_plane_change_flag;
int save_map_flag;
int load_map_flag;
int builder_spawn_flag;
int builder_delete_flag;

spawner_set* spawn_set = NULL;
int spawner_index;
compound_spawner* spawner_copy = NULL;
compound* spawned_item_copy = NULL;

compound* selected_item = NULL;
event* select_event = NULL;

void set_builder_selected_item(compound* sel) {
  fprintf(stderr, "builder selected an item!\n");
  if(sel==NULL){
    fprintf(stderr,"but its null...\n");
  }
  unset_select_visuals(selected_item);
  selected_item = sel;
  set_select_visuals(selected_item);
}

compound* get_builder_selected_item() {
  return selected_item;
}


void set_select_visuals(compound* comp) {
  if (comp == NULL) {
    return;
  }
  gen_node* curr = list_get_start(get_bodies(comp));
  body* curr_b = NULL;
  flags* curr_f = NULL;
  while (curr != NULL) {
    curr_b = (body*)list_get_data(curr);
    curr_f = body_get_draw_flags(curr_b);

    set_draw_outline(curr_f, 1);

    curr = list_get_next(curr);
  }
}

void unset_select_visuals(compound* comp) {
  if (comp == NULL) {
    return;
  }
  gen_node* curr = list_get_start(get_bodies(comp));
  body* curr_b = NULL;
  flags* curr_f = NULL;
  while (curr != NULL) {
    curr_b = (body*)list_get_data(curr);
    curr_f = body_get_draw_flags(curr_b);

    set_draw_outline(curr_f, 0);

    curr = list_get_next(curr);
  }
}

map* spawner_map;

void inc_spawner_index() {
  if (spawn_set == NULL) {
    return;
  }
  spawner_index++;
  if (spawner_index == get_spawner_set_len(spawn_set)) {
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
    spawner_index = get_spawner_set_len(spawn_set) - 1;
  }
  replace_spawned_copy();
}

char* spawner_entry() {
  return get_name_from_idx(spawn_set, spawner_index);
}

void set_spawner_set(spawner_set* ss) {
  if (ss == NULL) {
    return;
  }
  spawner_index = 0;
  spawn_set = ss;
  replace_spawned_copy();
}

gen_list* plane_list;
gen_node* plane_nd;


plane* plane_entry() {
  return (plane*)plane_nd->stored;
}

void set_plane_list(gen_list* list) {
  plane_list = list;
  plane_nd = plane_list->start;
}

void next_plane() {
  plane_nd = plane_nd->next;
  if (plane_nd == NULL) {
    plane_nd = plane_list->start;
  }
  clear_event_collider(select_event);
  init_event_collider(get_shm(plane_entry()), select_event);
}

void prev_plane() {
  plane_nd = plane_nd->prev;
  if (plane_nd == NULL) {
    plane_nd = plane_list->end;
  }
  clear_event_collider(select_event);
  init_event_collider(get_shm(plane_entry()), select_event);
}



void replace_spawned_copy() {
  plane* main_plane = (plane*)get_planes(spawner_map)->start->stored;
  virt_pos cent = get_body_center(get_compound_head(getBuilder()));
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



void builder_logic(map* m) {
  static map* prev_m = NULL;
  plane* pl = NULL;
  virt_pos cent;
  compound_spawner* spawner = NULL;
  int NAME_LEN = 30;
  char map_name[NAME_LEN];
  compound* builder = getBuilder();
  if (builder == NULL) {
    spawner_map = create_map("builder_map");
    add_plane(spawner_map, create_plane(create_shm(100,100,1,1) , "main_plane"));
    
    builder = mono_compound(makeNormalBody(13, 9));
    make_compound_builder(builder);
    center_cam_on_body(get_compound_head(builder));
    select_event = make_builder_select_event(get_compound_head(builder));
    set_spawner_set(main_set);
  }
  if (prev_m != m) {
    set_plane_list(get_planes(m));
    prev_m = m;
    clear_event_collider(select_event);
    init_event_collider(get_shm(plane_entry()), select_event);
  }
  check_event(get_shm(plane_entry()) ,select_event);
  run_body_poltergeist(get_compound_head(builder));
  cent = get_body_center(get_compound_head(builder));
  if (builder_spawn_flag) {
    spawner = create_compound_spawner(spawner_entry(), -1, cent.x, cent.y);
    pl = plane_entry();
    add_spawner_to_plane(pl, spawner);
    trigger_spawner(spawner, pl);
    builder_spawn_flag = 0;
  }
  if (builder_delete_flag) {
    pl = plane_entry();
    if (selected_item != NULL) {
      remove_compound_from_plane(pl, selected_item);
      compound_spawner* cs = get_spawner_p(selected_item);
      if (cs != NULL) {
	if (!remove_spawner_from_plane(pl, cs)) {
	  fprintf(stderr, "Warning, removed item from map but not the spawner\n");
	}
      }
      selected_item = NULL;
    }
    builder_delete_flag = 0;
  }
  if (save_map_flag) {
    fprintf(stdout, "input filename of map to save, max of %i characters\n", NAME_LEN);
    fgets(map_name, NAME_LEN, stdin);
    if (strlen(map_name) > 1) {
      map_name[strlen(map_name) - 1] = '\0';
    }
    set_map_name(m, map_name);
    save_map(m, map_name);
    save_map_flag = 0;
  }
  if (load_map_flag) {
    fprintf(stdout, "input filename of map to load, max of %i characters\n", NAME_LEN);
    fgets(map_name, NAME_LEN, stdin);
    if (strlen(map_name) > 1) {
      map_name[strlen(map_name) - 1] = '\0';
    }
    map* temp = load_map(map_name);
    plane* m_p = get_plane_by_name(temp, MAIN_PLANE_NAME);
    if (temp != NULL) {
      setMap(temp);
    }
    add_compound_to_plane(m_p, getUser());
    load_map_flag = 0;
  }
  check_events(get_shm(plane_entry()), get_body_events(get_compound_head(builder)));
  body* head = get_compound_head(spawned_item_copy);
  set_body_center(head, &cent);
  play_logic(spawner_map);
  //draw_compound(getCam(), builder);
  draw_compound(getCam(), spawned_item_copy);
  if (selected_item != NULL) {
    draw_compound(getCam(), selected_item);
  }

  //unset selected flags after draw
  unset_select_visuals(selected_item);
}

/*
  basic inputs
  normal arrow keys for movement

  ctrl inputes
  left/right changes active spawn item
  up/down changes active map plane
  m switches into/out of builder mode
  l loads map
  s saves map
  b moves user to builder
  u moves builder to user
  d deletes selected item
 */
int builder_input(body* b, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  int up = 0, down = 0, left = 0, right = 0;
  int spawner = 0, spawn_idx_inc = 0, spawn_idx_dec = 0, plane_next = 0, plane_prev = 0;
  int load_map = 0, save_map = 0, to_user = 0, to_builder = 0, delete = 0;
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
	case SDLK_l:
	  load_map = 1;
	  break;
	case SDLK_s:
	  save_map = 1;
	  break;
	case SDLK_b:
	  to_builder = 1;
	  break;
	case SDLK_u:
	  to_user = 1;
	  break;
	case SDLK_d:
	  delete = 1;
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

  if (save_map) {
    save_map_flag = 1;
  }
  if (load_map) {
    load_map_flag = 1;
  }

  t_vp = get_body_center(b);
  virt_pos_add(&t_vp, &offset, &t_vp);
  set_body_center(b, &t_vp);

  if (to_user) {
    body* user_head = get_compound_head(getUser());
    virt_pos user_p = get_body_center(user_head);
    set_compound_position(getBuilder(), &user_p);
  }
  else if (to_builder) {
    body* builder_head = get_compound_head(getBuilder());
    virt_pos build_p = get_body_center(builder_head);
    set_compound_position(getUser(), &build_p);
  }

  if (delete) {
    builder_delete_flag = 1;
  }
  
  setQuit(quit);
  return quit;
}
