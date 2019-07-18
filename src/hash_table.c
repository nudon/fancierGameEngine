#include <stdlib.h>
#include <math.h>
#include "hash_table.h"

//closed indexing/open hashing hash table
//using linear probing

#define TABLE_SAT_THRESH 0.6

struct hash_table_struct {
  //table size is a power of 2
  int table_power;
  int filled_indexes;
  double max_table_saturation;
  
  void** array;  
  int (*compare)(void* a, void* b);
  int (*raw_hash)(void* a);
};

hash_table* create_hash_table( int (*compare_func)(void* a, void* b), int (*raw_hash_func)(void*a), int power) {
  hash_table* ret = malloc(sizeof(hash_table));
  init_hash_table(ret, compare_func, raw_hash_func, power, TABLE_SAT_THRESH);
  return ret;
}

void init_hash_table(hash_table* table, int (*compare_func)(void* a, void* b), int (*raw_hash_func)(void*a), int power, double sat_thresh) {
  int size = 0;
  if (power <= 0) {
    power = 4;
  }
  sat_thresh = fmin(fabs(sat_thresh), 1);
  
  table->table_power = power;
  size = get_table_size(table);
  table->array = malloc(sizeof(void*) * size);
  for (int i = 0; i < size; i++) {
    table->array[i] = NULL;
  }
  table->filled_indexes = 0;
  table->max_table_saturation = sat_thresh;

  table->compare = compare_func;
  table->raw_hash = raw_hash_func;
}

int get_table_size(hash_table* t) {
  return 1 << t->table_power;
}

//returns negative 1 if data is already in table
//otherwise returns value of index to put data
int calc_data_index(hash_table* table, void* data) {
  void* curr_data = NULL;
  int index = index_hash(table, data);
  int size = get_table_size(table);
  curr_data = table->array[index];
  
  while(curr_data != NULL) {
    if (table->compare(data, curr_data) == 0) {
      return -1;
    }
    index = (index + 1) % size;
    curr_data = table->array[index];
  }
  return index;
}

//returns 1 if data was added
//returns 0 otherwise
int insert(hash_table* table, void* data) {
  int ret = 0;
  int index = calc_data_index(table, data);
  if (index >= 0) {
    if ((double)(table->filled_indexes + 1) / get_table_size(table) >= table->max_table_saturation) {
      while ((double)(table->filled_indexes + 1) / get_table_size(table) >= table->max_table_saturation) {
	grow_table(table);
      }
      index = calc_data_index(table, data);
    }
    table->array[index] = data;
    table->filled_indexes++;
    ret = 1;
  }
  return ret;
}

void grow_table(hash_table* t) {
  int old_size = get_table_size(t);
  int new_size = old_size * 2;
  void** new_array = malloc(sizeof(void*) * new_size);
  void** old_array = t->array;
  int index = 0;
  t->array = new_array;
  t->table_power++;
  for (int i = 0; i < new_size; i++) {
    new_array[i] = NULL;    
  }
  for (int i = 0; i < old_size; i++) {
    if (old_array[i] != NULL) {
      index = calc_data_index(t, old_array[i]);
      new_array[index] = old_array[i];
    }
  }
  free(old_array);

}


void clear_table(hash_table* table) {
  int size = get_table_size(table);
  for (int i = 0; i < size; i++) {
    table->array[i] = NULL;
  }
  table->filled_indexes = 0;
}


// 2^m = bucket_size of hash table
int mult_shift_hash(int x, int m) {
  static size_t a = 0;
  static int w = 0;
  size_t val = 0;
  if (a == 0) {
    w = sizeof(size_t) * 8;
    a = (1 << (w - 1)) / 4 + 1;
  }
  val = (size_t) (a*x) >> (w-m);
  return (int)val;
}

int simple_hash(hash_table* table, void* data) {
  return table->raw_hash(data) % get_table_size(table);
}

int index_hash(hash_table* table, void* data) {
  //call one of the hash implementations
  return mult_shift_hash(table->raw_hash(data), table->table_power);
}
