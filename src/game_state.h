#ifndef FILE_GAME_STATE_SEEN
#define FILE_GAME_STATE_SEEN

#define PLAY_MODE 1
#define BUILD_MODE 11

#include "map.h"

void setQuit(int new);
int getQuit();

void setMode(int new);
int getMode();

map* getMap();
void setMap(map* new);

void setCam(camera* cam);
camera* getCam();

compound* getUser();
void setUser(compound* new);

compound* getBuilder();
void setBuilder(compound* new);


#endif
