#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "myList.h"
#include "creations.h"
#include "util.h"

void flush_travel_list_for_map(map* map);
void flush_travel_list_for_plane(gen_list* compound_list, plane* dest);

struct map_struct {
  gen_list* planes_in_map;
  char* map_name;
};

map* current_map = NULL;

map* create_map(char* name) {
  map* new = malloc(sizeof(map));
  new->planes_in_map = createGen_list();
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
  prependToGen_list(get_planes(map), createGen_node(plane));
}

char* get_map_name(map* m) { return m->map_name; }


map* getMap() {
  return current_map;
}

void setMap(map* new) {
  //might want to eventually do some save routine
  current_map = new;
}


//map loading warzone

struct load_zone_struct {
  //can either have pointers or strings for indicating a map
  //strings if I want to use less memory but need a queing system for map
  //pointers if I can have every map loaded at the same time
  char* from_map;
  char* to_map;
  //similar system for planes
  char* from_plane;
  char* to_plane;
  //then a dest position
  virt_pos dest;
  //then hold an event struct for fldkasdf
  event* trigger;
};

//probably best to make a custom hallway/door object to hold specifics for source/dest connections, and events for boundary, then have planes hold a list of boundaries
//also trying to figure out how to get it in an easy way to save to disk
//easiest for that is to probably load maps in memory and reference them by string names

//okay made everything as if I was going to keep maps always loaded in
//but I realized that when creating the load_zone's i'd have to be getting planes out of already existing maps
//meaning I'd need to do string matching for maps and planes
//at that point might as well just implement the loading lists to save memory
//wait this will be annoyting, will have a char*** for [map][plane] things
//wait worse than that
//will have to have a char** | gen_list *** first level map for [map]
//then a second level char** | gen_list **  for matching plane names

//initializes outer most_maps
void map_load_init();

//initializes innter_maps to contain map/plane keys
void map_load_insert_travel_list(char* map_name, char* plane_name);
//returns the gen list for map/plane keys
gen_list* map_load_get_travel_list(char* map_name, char* plane_name);


#define MAP_LIM 16
#define PLANE_LIM 16

//contains map_name to index 
char* map_names[MAP_LIM];
//may have types for these fucked up, may need[val][];
//use index of map name to get plane names to index
char** plane_names_in_map[MAP_LIM];
//use map and plane index to get a gen list
gen_list** gen_lists_for_plane_in_map [MAP_LIM];

//arean't actually allocated here, just prototypes for the second layer types
//char** plane_names[PLANE_LIM];
//gen_list** gen_lists_for_plane[PLANE_LIM];

void init_map_load() {
  null_init_array((void**)map_names, MAP_LIM);
  null_init_array((void**)plane_names_in_map, MAP_LIM);
  null_init_array((void**)gen_lists_for_plane_in_map, MAP_LIM);
}

void map_load_create_travel_lists(map* map) {
  printf("generating travel list for map %s\n", get_map_name(map));
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
    plane_i = first_empty_index(plane_names, PLANE_LIM);
    plane_names[plane_i] = strdup(plane_name);

    gen_lists_for_planes = gen_lists_for_plane_in_map[map_i];
    gen_lists_for_planes[plane_i] = createGen_list();
  }
  else {
    fprintf(stderr, "Duplicate insertion into travel list: map=%s, plane=%s\n", map_name, plane_name);
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

void trigger_map_change(load_zone* lz, compound* trav) {
  printf("something is changing maps...\n");
  decision_att* att = get_attributes(trav);
  plane* curr_plane = NULL;
  gen_list* move = NULL;
  if (is_travel(att)) {
    //move to new map
    //will have to update this
    //can either use the compounds valid bit to remove from whereever it is
    //and add it to the loading list
    //or do more obxnoxious stuff that I can't even think of
    curr_plane = get_plane_by_name(getMap(), get_lz_from_plane(lz));
    remove_compound_from_plane(curr_plane, trav);
    move = map_load_get_travel_list(get_lz_to_map(lz), get_lz_to_plane(lz));
    appendToGen_list(move, createGen_node(trav));
    //also need to set trav's position
    set_compound_position(trav, &(lz->dest));
    if (is_user(att)) {
      //potentiall update global map so map user is on get's drawn
      if (lz->from_map != lz->to_map) {
	//update global map
	//should be safe to do so now
	//only dangerous if I trigger 2 loading zones at the same time
	//load new map and set
	map* newMap = NULL;
	newMap = load_map_by_name(lz->to_map);
	setMap(newMap);
	//then flush travel lists to new one
	flush_travel_list_for_map(newMap);
	//also generate travel lists for new map
	map_load_create_travel_lists(newMap);
      }
    }
  }
}

void flush_travel_list_for_map(map* map) {
  //go through all planes in map and flush traveel lists
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
  compound* comp = NULL;
  while(curr != NULL) {
    comp = (compound*)curr->stored;
    add_compound_to_plane(dest, comp);
    curr = curr->next;
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
      initGen_list(&hits);
      store_event_triggers(get_shm(curr_plane), anEvent, &hits);
      aHit = hits.start;
      while(aHit != NULL) {
	hit_poly = get_polygon((collider*)(aHit->stored));
	comp = get_owner(((collider*)aHit->stored)->body);
	//map change might free aHit, do this before calling
	aHit = aHit->next;
	if (do_polygons_intersect(hit_poly, event_poly)) {
	  //call trigger map change
	  trigger_map_change(lz, comp);
	}
	
      }
      clean_collider_list(&hits);
      curr_lz_node = curr_lz_node->next;
    }
    curr_plane_node = curr_plane_node->next;
  }
}

