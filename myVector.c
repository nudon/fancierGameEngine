#include <stdio.h>
#include "myVector.h"

static int shiftLeft(vector* vec, int start, int amount);

static int shiftRight(vector* vec, int start, int amount);

static int DEFAULT_SIZE = 10;
vector* newVector() {
  return newVectorOfSize(DEFAULT_SIZE);
}

vector* newVectorOfSize(int size) {
  vector* new = malloc(sizeof(vector));
  new->elements = malloc(sizeof(void*) * size);
  new->max_size = size;
  new->cur_size = 0;
  return new;
}

void free_vector(vector* vec) {
  free(vec->elements);
  free(vec);
}


int unique_add(vector* vec, void* new, comparer* comp) {
  int aiv = already_in_vector(vec, new, comp);
  if (!aiv) {
    addElement(vec, new);
  }
  return aiv;
}


comparer default_comp = {.compare = default_compare};

int default_compare(void* p1, void* p2) {
  int ret;
  if (p1 == p2) {
    ret = 0;
  }
  if (p1 > p2) {
    ret = 1;
  }
  else {
    ret = -1;
  }
  return ret;
}

int already_in_vector(vector* vec, void* new, comparer* comp) {
  int done = 0, i = 0, ret = 0;
  void* temp;
  if (comp == NULL) {
    //probably just have some default pointer comparer
    comp = &default_comp;
  }
  while(!done) {
    temp = elementAt(vec, i);
    if (temp == NULL) {
      done = 1;
      ret = 0;
    }
    else if (comp->compare(temp, new) == 0) {
      done = 1;
      ret = 1;
    }
    i++;
  }
  return ret;
}

void addElement(vector* vec, void* new) {
  if (vec->cur_size > vec->max_size) {
    grow_vector(vec);
  }
  vec->elements[vec->cur_size] = new;
  vec->cur_size++;
}

void setElementAt(vector* vec, int index, void* newitem) {
  if (index < vec->max_size) {
    if (index < vec->cur_size) {
      //potentially dumb
      fprintf(stderr, "overwriting a good(?) pointer\n");
    }
    vec->elements[index] = newitem;
  }
  else {
    //you dumb dumb
    fprintf(stderr, "setting element beyond max size of array, did nothin\n");
  }
}


typedef
struct {
  void (*copy)(void* v1, void* v2);
} copyer;
/*


void matrix_index_copy(matrix_index* m1, matrix_index* m2) {
  m1 = *m2;
}

void matrix_index_copy_wrapper(void* v1, void* v2) {
  matrix_index_copy((matrix_index*)v1, (matrix_index*)v2);
}

copyer matrix_index_copyer = {.copy = matrix_index_copy_wrapper};
*/
//can also be replecated by calling *(cast*)elementAt(asdfadf) = thing
void copyTo(vector* vec, void* new, int index, copyer* copy) {
  void* item = elementAt(vec, index);
  copy->copy(item, new);
}

int unique_copy(vector* vec, void* new, comparer* comp, copyer* copy) {
  int aiv = already_in_vector(vec, new, comp);
  if (!aiv) {
    grfom(vec);
    copyTo(vec, new, vec->cur_size, copy);
    vec->cur_size++;
  }
  return aiv;
}

int addElementAt(vector* vec, void* new, int index) {
  int ret = 0;
  ret = shiftRight(vec, index, 1);
  while (ret == 0) {
    grow_vector(vec);
    ret = shiftRight(vec, index, 1);
  }
  vec->elements[index] = new;
  vec->cur_size++;
  return ret;
}

void clearVector(vector* vec) {
  vec->cur_size = 0;
}

void* removeElementAt(vector* vec, int index) {
  void* rm = elementAt(vec, index);
  shiftLeft(vec, index + 1, 1);
  vec->cur_size--;
  return rm;
}

void* elementAt(vector* vec, int index) {
  if (index >= 0 && index < vec->cur_size) {
    return vec->elements[index];
  }
  else {
    return NULL;
  }
}

static int shiftLeft(vector* vec, int start, int amount) {
  int ret = 0;
  if (start - amount >= 0) {
    ret = 1;
    for (int i = start; i < vec->cur_size; i++) {
      vec->elements[i - amount] = vec->elements[i];
    }
  }
  return ret;
}

static int shiftRight(vector* vec, int start, int amount) {
  int ret = 0;
  if (vec->cur_size + amount <= vec->max_size) {
    ret = 1;
    for (int i = vec->cur_size - 1; start <= i; i--) {
      vec->elements[i + amount] = vec->elements[i];
    }
  }
    return ret;
}

void grow_vector(vector* vec) {
  int old_max_size = vec->max_size;
  int new_max_size = old_max_size * 2;
  void** temp = malloc(sizeof(void*) * new_max_size);
  for (int i = 0; i < new_max_size; i++) {
    temp[i] = NULL;
    if (i < old_max_size) {
      temp[i] = vec->elements[i];
    }
  }
  free(vec->elements);
  vec->elements = temp;
  vec->max_size = new_max_size;
}

//get ready for one more
void grfom(vector* vec) {
  if (vec->cur_size == vec->max_size) {
    grow_vector(vec);
  }
}
