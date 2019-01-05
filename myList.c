
#include "myList.h"


//so , want to add areferemce from nodes to   contained list

//and also a function to only add something to list if data/stored is a unique pointer


void appendToGen_list(gen_list* list, gen_node* new) {
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

void prependToGen_list(gen_list* list, gen_node* new) {
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

void removeFromGen_list(gen_list* list, gen_node* node) {
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

int add_if_unique_data(gen_list* list, gen_node* node) {
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
    appendToGen_list(list, node);
  }
  return unique;
}

int already_in_a_list(gen_node* node) {
  return node->list != NULL;
}

void remove_node(gen_node* node) {
  if (already_in_a_list(node)) {
    removeFromGen_list(node->list, node);
  }
}

gen_list* createGen_list() {
  gen_list* new = malloc(sizeof(gen_list));
  initGen_list(new);
  return new;
}

void initGen_list(gen_list* new) {
  new->start = NULL;
  new->end = NULL;
}

gen_node* createGen_node(void* data) {
  gen_node* new = malloc(sizeof(gen_node));
  new->next = NULL;
  new->prev = NULL;
  new->list = NULL;
  new->stored = data;
  return new;
}

void freeGen_node(gen_node* old) {
  free(old->stored);
  free(old);
}

void freeGen_list(gen_list* old) {
  gen_node* temp;
  gen_node* current;
  current = old->start;
  while(current != NULL) {
    temp = current;
    current = temp->next;
    freeGen_node(temp);
  }
  free(old);
}





