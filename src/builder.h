#ifndef FILE_BUILDER_FOUND
#define FILE_BUILDER_FOUND
#include "map.h"

/*
  holds special logic and state to allow a sort of map editing mode

  basic inputs
  normal arrow keys for movement

  ctrl inputes
  left/right changes active spawn item
  up/down changes active map plane
  m switches into/out of builder mode
  l loads map
  s saves map
  b moves user to builder
  u moves builder to user
*/

//main function that updates building variables and edits the map
void builder_logic(map* m);

//checks input and sets various builder.h flags
int builder_input(body* b, vector_2* trans_disp, double* rot_disp);

#endif
