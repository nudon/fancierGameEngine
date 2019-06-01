//kind of a skeleton for now
//in future, want to include compounds which are just collections of bodies
//main use/idea is that collisions between objects from same collections are ignored
//can suport multi-segmented things like centipides or the such
#include "compound.h"


struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
  vector_2 move_dir;
  decision_att* decision_attributes;
  //flag for removal of compound from plane
  int valid;
};
  

compound* create_compound() {
  compound* new = malloc(sizeof(compound));
  new->bp = createGen_list();
  new->move_dir = *zero_vec;
  new->decision_attributes = make_decision_att();
  new->valid = -1;
  return new;
}

gen_list* get_bodies(compound* comp) { return comp->bp; }

decision_att* get_attributes(compound* comp) {
  return comp->decision_attributes;
}

void set_attributes(compound* comp, decision_att* na) {
  copy_atts(na, comp->decision_attributes);
}

void add_body_to_compound(compound* comp, body* body) {
  //probably do some check to make sure body isn't already linked to a compound
  if (body->owner != NULL) {
    fprintf(stderr, "warning, overwriting compound owner of a body\n");
  }
  body->owner = comp;
  appendToGen_list(comp->bp, createGen_node(body));
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
  return comp->move_dir;
}

void set_dir(compound* comp, vector_2* nv) {
  comp->move_dir.v1 = nv->v1;
  comp->move_dir.v2 = nv->v2;
}


void set_valid(compound* comp, int val) {
  comp->valid = val;
}

int is_valid(compound* comp) {
  return (get_valid(comp) > 0);
}

int get_valid(compound* comp) {
  return comp->valid;
}
