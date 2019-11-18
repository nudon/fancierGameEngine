//kind of a skeleton for now
//in future, want to include compounds which are just collections of bodies
//main use/idea is that collisions between objects from same collections are ignored
//can suport multi-segmented things like centipides or the such
#include "compound.h"
#include "gi.h"


#define NONUNIFORM_COMPOUND 0
#define UNIFORM_COMPOUND 1

struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
  gen_list* self_tethers;
  smarts* smarts;
};  

compound* create_compound() {
  compound* new = malloc(sizeof(compound));
  new->bp = createGen_list();
  new->self_tethers = createGen_list();
  //new->compound_intelligence = create_gi();
  new->smarts = NULL;
  return new;
}

body* get_compound_head(compound* c) {
  return (body*)c->bp->start->stored;
}

compound* mono_compound(body* b) {
  compound* c = create_compound();
  add_body_to_compound(c, b);
  return c;
}

int move_body(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp) {
  shared_input* si = get_shared_input(b);
  if (si != NULL) {
    add_to_shared_input(t_disp, r_disp, si);
    return 1;
  }
  else {
    return update(map, get_collider(b), t_disp, r_disp);
  }
}

void move_compound(spatial_hash_map* map, compound* c) {
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
  n = get_bodies(c)->end;
  //printf("\n");
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
      update(map, get_collider(b), &t_disp, r_disp);
    }
    n = n->prev;
  }
}

void add_body_to_compound(compound* comp, body* b) {
  //probably do some check to make sure body isn't already linked to a compound
  if (get_owner(b) != NULL) {
    fprintf(stderr, "warning, overwriting compound owner of a body\n");
  }
  set_owner(b, comp);
  virt_pos offset = *zero_pos;
  polygon* p = NULL;
  if (get_shared_input(b) != NULL && comp->bp->start != NULL) {
    p = get_polygon(get_collider((b)));
    offset = get_rotational_offset(b);
    set_rotation_offset(p, &offset);
  }
  if (comp->smarts != NULL) {
    add_smarts_to_body(b);
  }
  appendToGen_list(comp->bp, createGen_node(b));
}

//delete any shared inputs or tether forces stored within compound
void cut_compound(compound* c) {
  gen_node* curr = get_bodies(c)->start;
  gen_node* prev = NULL;
  body* b = NULL;
  tether* teth = NULL;
  while(curr != NULL) {
    b = (body*)curr->stored;
    un_set_shared_input(b);
    curr = curr->next;
  }
  
  curr = get_compound_tethers(c)->start;
  while(curr != NULL) {
    teth = (tether*)curr->stored;
    //compound tethers only tether within the compound, should be safe to free
    free_tether(teth);
    prev = curr;
    curr = curr->next;
    remove_node(prev);
    freeGen_node(prev);
  }
  //initGen_list(get_compound_tethers(c));
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

gen_list* get_bodies(compound* comp) { return comp->bp; }

void set_compound_gravity(compound* c, vector_2* grav) {
  gen_node* body_node = get_bodies(c)->start;
  body* aBody;
  while(body_node != NULL) {
    aBody = (body*)body_node->stored;
    set_gravity(get_fizzle(aBody), grav);
    set_gravity(get_base_fizzle(aBody), grav);    
    body_node = body_node->next;
  }
}

void set_compound_smarts(compound* c, smarts* sm) {
  c->smarts = sm;
}

smarts* get_compound_smarts(compound* c) {
  return c->smarts;
}

void make_compound_smart(compound* c) {
  if (c->smarts != NULL) {
    fprintf(stderr, "warning, overwriting compound smarts\n");
  }
  add_smarts_to_comp(c);
}
