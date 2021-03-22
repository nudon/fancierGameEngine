
#include "myList.h"


//so , want to add areferemce from nodes to   contained list

//and also a function to only add something to list if data/stored is a unique pointer


void list_append(gen_list* list, gen_node* new) {
  if (list->start == NULL) {
    list->start = new;
  }
  if (list->end != NULL) {
    list->end->next = new;
    new->prev = list->end;
  }
  else {
    new->prev = NULL;
    list->start = new;
  }
  list->end = new;
  new->next = NULL;
  if (new->list != NULL) {
    fprintf(stderr, "error, node added to second list, first list is disjoint\n");
  }
  new->list = list;
}

void list_prepend(gen_list* list, gen_node* new) {
  if (list->end == NULL) {
    list->end = new;
  }
  if (list->start != NULL) {
    list->start->prev = new;
    new->next = list->start;
  }
  else {
    new->next = NULL;
    list->end = new;
  }
  list->start = new;
  new->prev = NULL;
  if (new->list != NULL) {
    fprintf(stderr, "error, node added to second list, first list is disjoint\n");
  }
  new->list = list;
}

void list_remove(gen_list* list, gen_node* node) {
  if (node != NULL) {
    if (node->prev == NULL && node->next == NULL) {
      list->start = NULL;
      list->end = NULL;
    }
    else {
      if (node->prev == NULL) {
	list->start = node->next;
	node->next->prev = NULL;
      }
      else if (node->next == NULL ){
	list->end = node->prev;
	node->prev->next = NULL;
      }
      else {
	node->prev->next = node->next;
	node->next->prev = node->prev;
      }
    }
    node->next = NULL;
    node->prev = NULL;
    node->list = NULL;
  }
}

int list_unique_add(gen_list* list, gen_node* node) {
  gen_node* curr = list->start;
  int done = 0, unique = 1;
  while(!done && curr != NULL) {
    if (curr->stored == node->stored) {
      done = 1;
      unique = 0;
    }
    curr = curr->next;
  }
  if (unique) {
    list_append(list, node);
  }
  return unique;
}

int already_in_list(gen_node* node) {
  return node->list != NULL;
}

void remove_node(gen_node* node) {
  if (already_in_list(node)) {
    list_remove(node->list, node);
  }
}

gen_list* create_gen_list() {
  gen_list* new = malloc(sizeof(gen_list));
  init_gen_list(new);
  return new;
}

void init_gen_list(gen_list* new) {
  new->start = NULL;
  new->end = NULL;
}

gen_node* create_gen_node(void* data) {
  gen_node* new = malloc(sizeof(gen_node));
  new->next = NULL;
  new->prev = NULL;
  new->list = NULL;
  new->stored = data;
  return new;
}

void free_gen_node(gen_node* old) {
  free(old);
}

void free_gen_list(gen_list* old) {
  gen_node* temp;
  gen_node* current;
  current = old->start;
  while(current != NULL) {
    temp = current;
    current = temp->next;
    free_gen_node(temp);
  }
  free(old);
}

void list_set_data(gen_node* n, void* d) {
  n->stored = d;
}

void* list_get_data(gen_node* n) {
  return n->stored;
}

gen_node* list_get_start(gen_list* list) {
  return list->start;
}
gen_node* list_get_end(gen_list* list) {
  return list->end;
}

gen_node* list_get_next(gen_node* curr) {
  return curr->next;
}
gen_node* list_get_prev(gen_node* curr) {
  return curr->prev;
}
