//kind of a skeleton for now
//in future, want to include compounds which are just collections of bodies
//main use/idea is that collisions between objects from same collections are ignored
//can suport multi-segmented things like centipides or the such
#include "compound.h"
#include "gi.h"


#define NONUNIFORM_COMPOUND 0
#define UNIFORM_COMPOUND 1

virt_pos get_rotational_offset(body* b);

struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
  gi* compound_intelligence;
  //stuff for uniform movement of compound
  //flag that determines if body parts should be moved uniformly or not'
  int uniform_flag;
  virt_pos t_disp;
  int t_count;
  double r_disp;
  int r_count;
};
  

compound* create_compound() {
  compound* new = malloc(sizeof(compound));
  new->bp = createGen_list();
  new->compound_intelligence = create_gi();
  new->uniform_flag = NONUNIFORM_COMPOUND;
  // new->uniform_flag = UNIFORM_COMPOUND;
  new->t_disp = *zero_pos;
  new->t_count = 0;
  new->r_disp = 0;
  new->r_count = 0;
  return new;
}

int body_update(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp) {
  if (r_disp != 0) {
    int a = 4;
  }
  compound* c = get_owner(b);
  if (is_compound_uniform(c)) {
    virt_pos_add(t_disp, &(c->t_disp), &(c->t_disp));
    c->t_count++;
    c->r_disp += r_disp;
    c->r_count++;
    return 1;
  }
  else {
    return update(map, b->coll, t_disp, r_disp);
  }
}

void compound_update(spatial_hash_map* map, compound* c) {
  gen_node* n  = NULL;
  body* b = NULL;
  virt_pos avg_t_disp = *zero_pos;
  virt_pos orig_offset = *zero_pos;
  virt_pos curr_offset = *zero_pos;
  virt_pos t_disp = *zero_pos;
  polygon* p = NULL;
  polygon* head = get_polygon(get_collider(((body*)c->bp->start->stored)));
  double r_disp = 0;
  if (is_compound_uniform(c)) {
    if (c->t_count != 0) {
      avg_t_disp.x = c->t_disp.x / c->t_count;
      avg_t_disp.y = c->t_disp.y / c->t_count;
      c->t_count = 0;
      c->t_disp = *zero_pos;
    }
    if (c->r_count != 0) {
      r_disp = c->r_disp / c->r_count;
      c->r_count = 0;
      c->r_disp = 0;
    }
    //r_disp = 0.003;
    n = get_bodies(c)->start;
    while(n != NULL) {
      b = (body*)n->stored;
      orig_offset = *zero_pos;
      curr_offset = get_rotational_offset(b);
      t_disp = *zero_pos;
      p = get_polygon(get_collider(b));
      if (!isZeroPos(&(p->rotation_offset))) {
	//modify t_disp for poly so it looks like it wasn't rotated about it's center
	
	virt_pos_rotate(&(p->rotation_offset), get_rotation(head), &orig_offset);
	virt_pos_sub(&orig_offset, &curr_offset, &t_disp);
	virt_pos_sub(&avg_t_disp,&t_disp, &t_disp);
      }
      else {
	//no rotation offset, nothing special happens
	t_disp = avg_t_disp;
      }
      update(map, b->coll, &t_disp, r_disp);
      n = n->next;
    }
  }
  //non-uniform compounds have body parts handled in body_update
  //else {}
}

virt_pos get_rotational_offset(body* b) {
  compound* comp = get_owner(b);
  virt_pos head_center = *zero_pos;
  virt_pos curr_center = *zero_pos;
  virt_pos offset = *zero_pos;
  if (is_compound_uniform(comp) && comp->bp->start != NULL) {
    head_center = get_center(get_polygon(get_collider(((body*)comp->bp->start->stored))));
    curr_center = get_center(get_polygon(get_collider((b))));
    virt_pos_sub(&head_center, &curr_center, &offset);
  }
  return offset;
}

void add_body_to_compound(compound* comp, body* b) {
  //probably do some check to make sure body isn't already linked to a compound
  if (b->owner != NULL) {
    fprintf(stderr, "warning, overwriting compound owner of a body\n");
  }
  b->owner = comp;
  virt_pos offset = *zero_pos;
  polygon* p = NULL;
  if (is_compound_uniform(comp) && comp->bp->start != NULL) {
    p = get_polygon(get_collider((b)));
    offset = get_rotational_offset(b);
    set_rotation_offset(p, &offset);
  }
  appendToGen_list(comp->bp, createGen_node(b));
}

//join together bodies of comp
//teth, optional. if !NULL, copy setup from teth. else use a default
//genList, a list to append created tethers to.
void tether_join_compound(compound* comp, tether* teth, gen_list* append) {
  gen_node* curr_body = comp->bp->start;
  body* b1 = NULL, *b2 = NULL;
  tether* body_teth;
  while(curr_body != NULL) {
    b2 = b1;
    b1 = (body*)curr_body->stored;
    if (b2 != NULL) {
      body_teth = create_tether_blank(getCenter(b1),getCenter(b2),getFizzle(b1),getFizzle(b2));
      if (teth == NULL) {
	teth = default_tether;
      }
      copy_tether_params(teth, body_teth);
      if (append != NULL) {
	appendToGen_list(append, createGen_node(body_teth));
      }
    }
    curr_body = curr_body->next;
  }
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

void make_compound_uniform(compound* c) {
  c->uniform_flag = UNIFORM_COMPOUND;
}

int is_compound_uniform(compound* c) {
  return c->uniform_flag == UNIFORM_COMPOUND;
}

int get_compound_uniform_flag(compound* c) {
  return c->uniform_flag;
}

void set_compound_uniform_flag(compound* c, int v) {
  c->uniform_flag = v;
}
