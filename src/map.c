#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "myList.h"
#include "objects.h"
#include "map_io.h"
#include "util.h"
#include "game_state.h"

void flush_travel_list_for_map(map* map);
void flush_travel_list_for_plane(gen_list* compound_list, plane* dest);

struct map_struct {
  gen_list* planes_in_map;
  char* map_name;
};


map* create_map(char* name) {
  map* new = malloc(sizeof(map));
  new->planes_in_map = create_gen_list();
  new->map_name = strdup(name);
  return new;
}

plane* get_plane_by_name(map* m, char* name) {
  gen_node* curr = get_planes(m)->start;
  plane* temp = NULL;
  plane* ret = NULL;
  while(curr != NULL) {
    temp = (plane*)curr->stored;
    if (strcmp(get_plane_name(temp), name) == 0) {
      ret = temp;
      curr = NULL;
    }
    else {
      curr = curr->next;
    }
  }
  return ret;
}

gen_list* get_planes(map* map) {
  return map->planes_in_map;
}

void add_plane(map* map, plane* plane) {
  list_prepend(get_planes(map), create_gen_node(plane));
}

char* get_map_name(map* m) {
  return m->map_name;
}

void set_map_name(map* m, char* name) {
  if (m->map_name != NULL) {
    free(m->map_name);
  }
  m->map_name = strdup(name);
}


struct load_zone_struct {
  char* from_map;
  char* to_map;
  char* from_plane;
  char* to_plane;
  //then a dest position to place user in to_map/plane
  virt_pos dest;
  //event to trigger the map change
  event* trigger;
};

//initializes inner_maps to contain map/plane keys
void map_load_insert_travel_list(char* map_name, char* plane_name);
//returns the gen list for map/plane keys
gen_list* map_load_get_travel_list(char* map_name, char* plane_name);


#define MAP_LIM 16
#define PLANE_LIM 16

//contains map_name to index 
char* map_names[MAP_LIM];
//use index of map name to get plane names to index
char** plane_names_in_map[MAP_LIM];
//use map and plane index to get a gen list
gen_list** gen_lists_for_plane_in_map [MAP_LIM];


void init_map_load() {
  null_init_array((void**)map_names, MAP_LIM);
  null_init_array((void**)plane_names_in_map, MAP_LIM);
  null_init_array((void**)gen_lists_for_plane_in_map, MAP_LIM);
}

void map_load_create_travel_lists(map* map) {
  gen_node* curr_plane = get_planes(map)->start;
  plane* aPlane = NULL;
  gen_node* curr_lz = NULL;
  load_zone* lz = NULL;
  while(curr_plane != NULL) {
    aPlane = (plane*)curr_plane->stored;
    curr_lz = get_load_zones(aPlane)->start;
    while(curr_lz != NULL) {
      lz = (load_zone*)curr_lz->stored;
      map_load_insert_travel_list(get_lz_to_map(lz), get_lz_to_plane(lz));
      curr_lz = curr_lz->next;
    }
    curr_plane = curr_plane->next;
  }
}

void map_load_insert_travel_list(char* map_name, char* plane_name) {
  int map_i = char_search(map_names, map_name, MAP_LIM);
  int plane_i = -1;
  char** plane_names = NULL;
  gen_list** gen_lists_for_planes = NULL;
  if (map_i < 0) {
    map_i = first_empty_index(map_names, MAP_LIM);
    map_names[map_i] = strdup(map_name);

    plane_names = malloc(sizeof(char*) * PLANE_LIM);
    null_init_array((void**)plane_names, PLANE_LIM);
    plane_names_in_map[map_i] = plane_names;
    
    gen_lists_for_planes = malloc(sizeof(gen_list*) * PLANE_LIM);
    null_init_array((void**)gen_lists_for_planes, PLANE_LIM);
    gen_lists_for_plane_in_map[map_i] = gen_lists_for_planes;
  }
  
  plane_names = plane_names_in_map[map_i];
  plane_i = char_search(plane_names, plane_name, PLANE_LIM);
  if (plane_i < 0) {
    printf("generating travel list for map %s\n", map_name);
    plane_i = first_empty_index(plane_names, PLANE_LIM);
    plane_names[plane_i] = strdup(plane_name);

    gen_lists_for_planes = gen_lists_for_plane_in_map[map_i];
    gen_lists_for_planes[plane_i] = create_gen_list();
  }
  else {
    //travel lists have already been created
  }
}

gen_list* map_load_get_travel_list(char* map_name, char* plane_name) {
  int map_i = char_search(map_names, map_name, MAP_LIM);
  char** plane_names = NULL;
  gen_list* ret = NULL;
  if (map_i >= 0) {
    plane_names = plane_names_in_map[map_i];
    int plane_i = char_search(plane_names, plane_name, PLANE_LIM);
    if (plane_i >= 0) {
      ret = gen_lists_for_plane_in_map[map_i][plane_i];
    }
  }
  return ret;
}

load_zone* make_load_zone(char* m_from, char* m_to, char* p_from, char* p_to, virt_pos* dest, event* trigger) {
  load_zone* new = malloc(sizeof(load_zone));
  new->from_map = strdup(m_from);
  new->to_map = strdup(m_to);
  new->from_plane = strdup(p_from);
  new->to_plane = strdup(p_to);
  new->dest = *dest;
  new->trigger = trigger;
  return new;
}

//load_zone functions

char* get_lz_from_map(load_zone* lz) { return lz->from_map;}
char* get_lz_to_map(load_zone* lz) { return lz->to_map; }
char* get_lz_from_plane(load_zone* lz) { return lz->from_plane; }
char* get_lz_to_plane(load_zone* lz) { return lz->to_plane; }

virt_pos get_lz_dest(load_zone* lz) { return lz->dest; }
event* get_lz_event(load_zone* lz) { return lz->trigger; }


#define MAP_NOCHANGE 0
#define MAP_CHANGE 1
int trigger_map_change(load_zone* lz, compound* trav) {
  att* att = get_comp_attributes(get_compound_smarts(trav));
  plane* curr_plane = NULL;
  gen_list* move = NULL;
  int ret = MAP_NOCHANGE;
  if (is_travel(att)) {
    ret = MAP_CHANGE;
    curr_plane = get_plane_by_name(getMap(), get_lz_from_plane(lz));
    remove_compound_from_plane(curr_plane, trav);
    move = map_load_get_travel_list(get_lz_to_map(lz), get_lz_to_plane(lz));
    list_append(move, create_gen_node(trav));
    set_compound_position(trav, &(lz->dest));
    if (is_user(att)) {
      if (lz->from_map != lz->to_map) {
	//update global map
	map* newMap = NULL;
	newMap = load_map(lz->to_map);
	setMap(newMap);
	map_load_create_travel_lists(newMap);
      }
    }
  }
  return ret;
}

void flush_travel_list_for_map(map* map) {
  int map_i = char_search(map_names, get_map_name(map), MAP_LIM);
  int plane_i = 0;
  char* plane_name = NULL;
  plane* aPlane = NULL;
  gen_node* curr = get_planes(map)->start;
  if (map_i >= 0) {
    gen_list** lists_to_flush = gen_lists_for_plane_in_map[map_i];
    while(curr != NULL) {
      aPlane = (plane*)curr->stored;
      plane_name = get_plane_name(aPlane);
      plane_i = char_search(plane_names_in_map[map_i], plane_name, PLANE_LIM);
      if (plane_i >= 0) {
	flush_travel_list_for_plane(lists_to_flush[plane_i], aPlane);
      }
      curr = curr->next;
    }
  }
}

void flush_travel_list_for_plane(gen_list* compound_list, plane* dest) {
  gen_node* curr = compound_list->start;
  gen_node* temp = NULL;
  compound* comp = NULL;
  while(curr != NULL) {
    comp = (compound*)curr->stored;
    add_compound_to_plane(dest, comp);

    temp = curr;
    curr = curr->next;
    remove_node(temp);
    free_gen_node(temp);
  }
}

void check_load_triggers(map* map) {
  gen_node* curr_plane_node = get_planes(map)->start;
  gen_node* curr_lz_node = NULL;
  plane* curr_plane = NULL;
  gen_node* aHit = NULL;
  gen_list hits;
  load_zone* lz = NULL;
  event* anEvent = NULL;
  polygon* event_poly = NULL;
  polygon* hit_poly = NULL;
  compound* comp = NULL;
  while(curr_plane_node != NULL) {
    curr_plane = (plane*)curr_plane_node->stored;
    curr_lz_node = get_load_zones(curr_plane)->start;
    while(curr_lz_node != NULL) {
      lz = (load_zone*)curr_lz_node->stored;
      anEvent = lz->trigger;
      event_poly = get_polygon(get_event_collider(anEvent));
      init_gen_list(&hits);
      store_event_triggers(get_shm(curr_plane), anEvent, &hits);
      aHit = hits.start;
      while(aHit != NULL) {
	hit_poly = get_polygon((collider*)(aHit->stored));
	comp = get_owner(((collider*)aHit->stored)->body);
	aHit = aHit->next;
	if (do_polygons_intersect(hit_poly, event_poly)) {
	  if (trigger_map_change(lz, comp) == MAP_CHANGE) {
	    //list changed, start over to avoid concurrent modification errors
	    aHit = hits.start;
	  }
	}
	
      }
      clean_collider_list(&hits);
      curr_lz_node = curr_lz_node->next;
    }
    curr_plane_node = curr_plane_node->next;
  }
}

void prep_for_save(map* m) {
  //do any special steps needed prior to saving map
  refund_spawners_in_map(m);
}

void prep_for_load(map* m) {
  //activate spawners 
  trigger_spawners_in_map(m);
  flush_travel_list_for_map(m);
}

void refund_spawners_in_map(map* m) {
  gen_node* curr_plane_node = get_planes(m)->start;
  gen_node* curr_comp_node = NULL;
  plane* curr_plane = NULL;
  compound* comp = NULL;
  while(curr_plane_node != NULL) {
    curr_plane = (plane*)curr_plane_node->stored;
    curr_comp_node = get_compounds(curr_plane)->start;
    while(curr_comp_node != NULL) {
      comp = (compound*)curr_comp_node->stored;
      if (get_spawner_p(comp) != NULL) {
	refund_spawner(get_spawner_p(comp));
	remove_compound_from_plane(curr_plane, comp);
      }
      curr_comp_node = curr_comp_node->next;
    }
    curr_plane_node = curr_plane_node->next;
  }
}
