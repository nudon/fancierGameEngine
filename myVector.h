#ifndef FILE_MYVECTOR_SEEN
#define FILE_MYVECTOR_SEEN

#include <stdlib.h>

typedef
struct {
  void** elements;
  int max_size;
  int cur_size;
} vector;

typedef
struct {
  int (*compare)(void* t1, void*t2);
}comparer;

int default_compare(void* p1, void* p2);


vector* newVector();

vector* newVectorOfSize(int size);

void free_vector(vector* vec);

int unique_add(vector* vec, void* new,comparer* comp);




int already_in_vector(vector* vec, void* new, comparer* comp);

void addElement(vector* vec, void* new);

void setElementAt(vector* vec, int index, void* newitem);

/*
typedef
struct {
  void (*copy)(void* v1, void* v2);
} copyer;

void matrix_index_copy(matrix_index* m1, matrix_index* m2);

void matrix_index_copy_wrapper(void* v1, void* v2);

void copyTo(vector* vec, void* new, int index, copyer* copy);

int unique_copy(vector* vec, void* new, copyer* copy);

*/

int addElementAt(vector* vec, void* new, int index);

void clearVector(vector* vec);

void* removeElementAt(vector* vec, int index);

void* elementAt(vector* vec, int index);

void grow_vector(vector* vec);

//get ready for one more
void grfom(vector* vec);
#endif
