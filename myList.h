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

void appendToGen_list(gen_list* list, gen_node* new);
void prependToGen_list(gen_list* list, gen_node* new);
void removeFromGen_list(gen_list* list, gen_node* node);

int add_if_unique_data(gen_list* list, gen_node* node);

int already_in_a_list(gen_node* node);

void remove_node(gen_node* node);

gen_list* createGen_list();

gen_node* createGen_node(void* data);

void freeGen_node(gen_node* old);

void freeGen_list(gen_list* old);

#endif
