//kind of a skeleton for now
//in future, want to include compounds which are just collections of bodies
//main use/idea is that collisions between objects from same collections are ignored
//can suport multi-segmented things like centipides or the such
#include "compound.h"
#include "objects.h"
#include "gi.h"


#define NONUNIFORM_COMPOUND 0
#define UNIFORM_COMPOUND 1



struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
  gen_list* self_tethers;
  smarts* smarts;
  compound_spawner* spawner;
};  

compound* create_compound() {
  compound* new = malloc(sizeof(compound));
  new->bp = create_gen_list();
  new->self_tethers = create_gen_list();
  //new->compound_intelligence = create_gi();
  new->smarts = NULL;
  new->spawner = NULL;
  return new;
}

body* get_compound_head(compound* c) {
  return (body*)c->bp->start->stored;
}

void set_spawner_p(compound* c, compound_spawner* cs) {
  c->spawner = cs;
}

compound_spawner* get_spawner_p(compound* c) {
  return c->spawner;
}

compound* mono_compound(body* b) {
  compound* c = create_compound();
  add_body_to_compound(c, b);
  return c;
}

void add_body_to_compound(compound* comp, body* b) {
  if (get_owner(b) != NULL) {
    fprintf(stderr, "warning, overwriting compound owner of a body\n");
  }
  set_owner(b, comp);
  
  if (comp->smarts != NULL) {
    add_smarts_to_body(b);
  }
  list_append(comp->bp, create_gen_node(b));
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
    free_gen_node(prev);
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
  if (teth == NULL) {
    teth = default_tether;
  }
  while(curr_body != NULL) {
    b2 = b1;
    b1 = (body*)curr_body->stored;
    
    if (b2 != NULL) {
      p1 = get_polygon(get_collider(b1));
      p2 = get_polygon(get_collider(b2));
      body_teth = create_tether_blank(read_only_polygon_center(p1),read_only_polygon_center(p2),get_fizzle(b1),get_fizzle(b2));
      copy_tether_params(teth, body_teth);
      
      add_tether_to_compound(comp, body_teth);
    }
    curr_body = curr_body->next;
  }
}

void add_tether_to_compound(compound* comp, tether* teth) {
  list_append(comp->self_tethers, create_gen_node(teth));
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
    set_body_gravity(aBody, grav);
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

void free_compound(compound* rm) {
  body* b = NULL;
  tether* t = NULL;
  gen_node* curr = rm->bp->start;
  while(curr != NULL) {
    b = (body*)curr->stored;
    free_body(b);
    curr = curr->next;
  }
  curr = rm->self_tethers->start;
  while(curr != NULL) {
    t = (tether*)curr->stored;
    free_tether(t);
    curr = curr->next;
  }
  free_compound_smarts(rm);
}
