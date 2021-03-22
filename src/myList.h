#ifndef FILE_MYLIST_SEEN
#define FILE_MYLIST_SEEN
#include <stdlib.h>
#include <stdio.h>
struct gen_list_struct;
struct gen_node_struct{
  void* stored;
  struct gen_node_struct * next;
  struct gen_node_struct * prev;
  struct gen_list_struct * list;
};

typedef struct gen_node_struct gen_node;

struct gen_list_struct {
  gen_node* start;
  gen_node* end;
};

typedef struct gen_list_struct gen_list;  

//not sure what to do here
//can either keep these static for internal use
//and just write seperate functions which take some specific datatype
//and do void* casting on their own
//or just let these be used anywhere
/*

*/

void list_append(gen_list* list, gen_node* new);
void list_prepend(gen_list* list, gen_node* new);
void list_remove(gen_list* list, gen_node* node);

int list_unique_add(gen_list* list, gen_node* node);

int already_in_list(gen_node* node);

void remove_node(gen_node* node);

gen_list* create_gen_list();

void init_gen_list(gen_list* new);

gen_node* create_gen_node(void* data);

void free_gen_node(gen_node* old);

void free_gen_list(gen_list* old);

void list_set_data(gen_node* n, void* d);
void* list_get_data(gen_node* n);

gen_node* list_get_start(gen_list* list);
gen_node* list_get_end(gen_list* list);
gen_node* list_get_next(gen_node* curr);
gen_node* list_get_prev(gen_node* curr);

#endif
