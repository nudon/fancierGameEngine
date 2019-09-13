//kind of a skeleton for now
//in future, want to include compounds which are just collections of bodies
//main use/idea is that collisions between objects from same collections are ignored
//can suport multi-segmented things like centipides or the such
#include "compound.h"
#include "gi.h"


#define NONUNIFORM_COMPOUND 0
#define UNIFORM_COMPOUND 1

typedef struct compound_stats_struct compound_stats;

struct compound_stats_struct {
  int max_jumps;
  int jumps_left;
  int jump_airtime;
  int max_health;
  int health;
};

struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
  gen_list* self_tethers;
  gi* compound_intelligence;
  compound_stats* c_stats;
};



void damage_compound(compound* c, double amt) {
  compound_stats* s = c->c_stats;
  if (s->health < amt) {
    s->health = 0;
    //do some on-death logic
  }
  else {
    s->health -= amt;
  }
}

  

compound* create_compound() {
  compound* new = malloc(sizeof(compound));
  new->bp = createGen_list();
  new->self_tethers = createGen_list();
  new->compound_intelligence = create_gi();
  new->c_stats = NULL;
  return new;
}

body* get_compound_head(compound* c) {
  return (body*)c->bp->start->stored;
}

int body_update(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp) {
  shared_input* si = get_shared_input(b);
  if (si != NULL) {
    add_to_shared_input(t_disp, r_disp, si);
    return 1;
  }
  else {
    return update(map, b->coll, t_disp, r_disp);
  }
}

void compound_update(spatial_hash_map* map, compound* c) {
  gen_node* n  = NULL;
  body* b = NULL;
  virt_pos rot_offset = *zero_pos;
  virt_pos curr_offset = *zero_pos;
  virt_pos orig_offset = *zero_pos;
  virt_pos t_disp = *zero_pos;
  polygon* head = get_polygon(get_collider(get_compound_head(c)));
  double r_disp = 0;
  virt_pos avg_t_disp = *zero_pos;
  double avg_r_disp = 0;
  shared_input* si = NULL;
  n = get_bodies(c)->start;
  while(n != NULL) {
    b = (body*)n->stored;
    si = get_shared_input(b);
    rot_offset = get_rotation_offset(get_polygon(get_collider(b)));
    if (si != NULL)  {
      get_avg_movement(si, &avg_t_disp, &avg_r_disp);
      orig_offset = *zero_pos;
      curr_offset = get_rotational_offset(b);
      t_disp = *zero_pos;
      if (!isZeroPos(&rot_offset)) {
	//modify t_disp for poly so it looks like it wasn't rotated about it's center
	
	virt_pos_rotate(&rot_offset, get_rotation(head), &orig_offset);
	virt_pos_sub(&orig_offset, &curr_offset, &t_disp);
	virt_pos_sub(&avg_t_disp,&t_disp, &t_disp);
      }
      else {
	//no rotation offset, nothing special happens
	t_disp = avg_t_disp;
      }
      r_disp = avg_r_disp;
      update(map, b->coll, &t_disp, r_disp);
    }
    n = n->next;
  }
}

void add_body_to_compound(compound* comp, body* b) {
  //probably do some check to make sure body isn't already linked to a compound
  if (b->owner != NULL) {
    fprintf(stderr, "warning, overwriting compound owner of a body\n");
  }
  b->owner = comp;
  virt_pos offset = *zero_pos;
  polygon* p = NULL;
  if (get_shared_input(b) != NULL && comp->bp->start != NULL) {
    p = get_polygon(get_collider((b)));
    offset = get_rotational_offset(b);
    set_rotation_offset(p, &offset);
  }
  appendToGen_list(comp->bp, createGen_node(b));
}

//join together bodies of comp
//teth, optional. if !NULL, copy setup from teth. else use a default
//genList, a list to append created tethers to.
void tether_join_compound(compound* comp, tether* teth) {
  gen_node* curr_body = comp->bp->start;
  body* b1 = NULL, *b2 = NULL;
  tether* body_teth;
  polygon* p1 = NULL;
  polygon* p2 = NULL;
  while(curr_body != NULL) {
    b2 = b1;
    b1 = (body*)curr_body->stored;
    
    p1 = get_polygon(get_collider(b1));
    p2 = get_polygon(get_collider(b2));
    if (b2 != NULL) {
      body_teth = create_tether_blank(read_only_polygon_center(p1),read_only_polygon_center(p2),get_fizzle(b1),get_fizzle(b2));
      if (teth == NULL) {
	teth = default_tether;
      }
      copy_tether_params(teth, body_teth);
      
      add_tether_to_compound(comp, body_teth);
    }
    curr_body = curr_body->next;
  }
}

void add_tether_to_compound(compound* comp, tether* teth) {
  appendToGen_list(comp->self_tethers, createGen_node(teth));
}

gen_list* get_compound_tethers(compound* comp) {
  return comp->self_tethers;
}

void set_compound_position(compound* comp, virt_pos* np) {
  gen_node* curr = get_bodies(comp)->start;
  body* aBody = NULL;
  while(curr != NULL) {
    aBody = (body*)curr->stored;
    set_center(get_polygon(get_collider(aBody)), np);
    curr = curr->next;
  }
}


vector_2 get_dir(compound* comp) {
  return get_curr_dir(get_gi(comp));
}

gen_list* get_bodies(compound* comp) { return comp->bp; }

decision_att* get_attributes(compound* comp) {
  return get_gi_attributes(get_gi(comp));
}

void set_attributes(compound* comp, decision_att* na) {
  copy_atts(na, get_gi_attributes(get_gi(comp)));
}

gi* get_gi(compound* comp) {
  return comp->compound_intelligence;
}

void set_gi(compound* comp, gi* g) {
  comp->compound_intelligence = g;
}

void set_compound_gravity(compound* c, vector_2* grav) {
  gen_node* body_node = get_bodies(c)->start;
  body* aBody;
  while(body_node != NULL) {
    aBody = (body*)body_node->stored;
    set_gravity(get_fizzle(aBody), grav);
    body_node = body_node->next;
  }
}
