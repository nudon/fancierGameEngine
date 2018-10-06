#include <stdlib.h>
#include <stdio.h>
#include "myMatrix.h"

gen_matrix* newGen_matrix(int cols, int rows) {
  gen_matrix* new = malloc(sizeof(gen_matrix));
  new->cols = cols;
  new->rows = rows;
  new->data = malloc(sizeof(void**) * rows);
  for (int rowIndex = 0; rowIndex < rows; rowIndex++) {
    new->data[rowIndex] = malloc(sizeof(void*) * cols);
    for(int colIndex = 0; colIndex < cols; colIndex++) {
      new->data[rowIndex][colIndex] = NULL;
    }
  }
  return new;
}

void freeGen_matrix(gen_matrix* rm) {
  //can't really free the actual elements within data, for obviouse reasons
  for (int rowIndex = 0; rowIndex < rm->rows; rowIndex++) {
    free(rm->data[rowIndex]);
  }
  free(rm->data);
  free(rm);
}

void* getDataAtIndex(gen_matrix* mat, int col, int row) {
  if (col < 0 || col >= mat->cols ||
      row < 0 || row >= mat->rows) {
    //fprintf(stderr, "Unable to access data in gen matrix at index col %d, row %d. oob\n", col, row);
    return NULL;
  }
  else {
    return mat->data[row][col];
  }
}

void setDataAtIndex(gen_matrix* mat, int col, int row, void* item) {
  if (col < 0 || col >= mat->cols ||
      row < 0 || row >= mat->rows) {
    //fprintf(stderr, "Unable to access data in gen matrix at index col %d, row %d. oob\n", col, row);
  }
  else {
    mat->data[row][col] = item;
  }  
}
