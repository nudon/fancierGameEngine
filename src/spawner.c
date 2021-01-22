#include "spawner.h"

static int search(spawner_set* s, char* name);

struct spawner_set_struct {
  int size;
  int i;
  char** names;
  compound* (**spawners)(void);
};

spawner_set* create_spawner_set(int len) {
  spawner_set* new = malloc(sizeof(spawner_set));
  new->size = len;
  new->i = 0;
  new->names = calloc(len, sizeof(char*));
  new->spawners = calloc(len, sizeof(void*));

  return new;
}
 
void spawner_set_append(spawner_set* ss, char* name, compound* (*func)(void)) {
  int i = ss->i;
  if (search(ss, name) != -1) {
    fprintf(stderr, "warning, duplicate name \"%s\"in spawner set\n", name);
    return;
  }
  if (i >= ss->size) {
    fprintf(stderr, "error, could not append to set, size limit reached\n");
    return;
  }
  
  ss->names[i] = strdup(name);
  ss->spawners[i] = func;
  ss->i++;
}
  

static int search(spawner_set* ss, char* name) {
  int i = 0;
  while(i < ss->i && strcmp(ss->names[i], name) != 0) {
    i++;
  }
  if (ss->names[i] == NULL) {
    return -1;
  }
  return i;
}

void get_func_from_name(spawner_set* ss, char* name, compound* (**func)(void)) {
  int i = search(ss, name);
  *func = NULL;
  get_func_from_idx(ss, i, func);
}

void get_func_from_idx(spawner_set* ss, int i, compound* (**func)(void)) {
  if (i >= 0 && i < ss->size) {
    *func = ss->spawners[i];
  }
}

char* get_name_from_idx(spawner_set* ss, int i) {
  if (i >= 0 && i < ss->size) {
    return ss->names[i];
  }
  return '\0';
}

int get_spawner_set_len(spawner_set* ss) {
  return ss->i;
}


struct compound_spawner_struct {
  char* compound_name;
  int spawn_cap;
  virt_pos spawn_center;
  compound* (*spawner) (void);
};

compound_spawner* create_compound_spawner(char* name, int cap, int x_pos, int y_pos) {
  compound_spawner* spawn = malloc(sizeof(compound_spawner));
  spawn->compound_name = strdup(name);
  spawn->spawn_cap = cap;
  spawn->spawn_center = (virt_pos){.x = x_pos, .y = y_pos};
  if (strcmp(name, BLUE_SLIME_SPAWN) == 0) {
    spawn->spawner = &makeSlime;
  }
  else if (strcmp(name, TEST_SPAWN) == 0) {
    //spawn->spawner = &tunctish;
    spawn->spawner = &monkey;
  }
  else if (strcmp(name, TRASHCAN_SPAWN) == 0) {
    spawn->spawner = &makeTrashCan;
  }
  else if (strcmp(name, GOHEI_SPAWN) == 0) {
    spawn->spawner = &makeGohei;
  }
  else if (strcmp(name, CEILING_GRASS_SHORT) == 0) {
    spawn->spawner = &ceiling_grass_short;
  }
  else if (strcmp(name, FLOOR_GRASS_SHORT) == 0) {
    spawn->spawner = &floor_grass_short;
  }
  else {
    fprintf(stderr, "warning, unable to find spawner for %s\n", name);
  }
  return spawn;
}

void free_compound_spawner(compound_spawner* rm) {
  free(rm);
}
 
char* get_spawner_name(compound_spawner* spawn) {
  return spawn->compound_name;
}

int get_spawner_cap(compound_spawner* spawn) {
  return spawn->spawn_cap;
}

void get_spawner_pos(compound_spawner* spawn, virt_pos* result) {
  *result = spawn->spawn_center;
}

void trigger_spawners_in_map(map* map) {
  gen_node* curr_plane = get_planes(map)->start;
  gen_node* curr_spawner;
  plane* p = NULL;
  compound_spawner* spawn = NULL;
  while(curr_plane != NULL) {
    p = (plane*)curr_plane->stored;
    curr_spawner = get_spawners(p)->start;
    while(curr_spawner != NULL) {
      spawn = (compound_spawner*)curr_spawner->stored;
      trigger_spawner(spawn, p);
      curr_spawner = curr_spawner->next;
    }
    curr_plane = curr_plane->next;
  }
}


compound* compound_from_spawner(compound_spawner* spawn) {
  compound* spawned = NULL;
  virt_pos pos = *zero_pos;
  if (spawn->spawn_cap != 0) {
    if (spawn->spawn_cap > 0) {
      spawn->spawn_cap--;
    }
    get_spawner_pos(spawn, &pos);
    spawned = spawn->spawner();
    set_spawner_p(spawned, spawn);
    offset_compound(spawned, &pos);
  }
  return spawned;
}

void trigger_spawner(compound_spawner* spawn, plane* insert) {
  compound* spawned = compound_from_spawner(spawn);
  if (spawned != NULL) { 
    add_compound_to_plane(insert, spawned);
  }
}

void refund_spawner(compound_spawner* spawn) {
  if (spawn == NULL) {
    return;
  }
  if (spawn->spawn_cap >= 0) {
    spawn->spawn_cap++;
  }
}
