#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>
#include <pthread.h>
#include "entities.h"

extern std::string gameEvent;
extern bool running;
extern bool gameOver;

extern Player player;
extern std::vector<Barrel> barrels;

extern pthread_mutex_t gameMutex;

extern char lastKey;

extern bool hammerActive;
extern bool hammerCollected;
extern int hammerTimer;

extern int levelGame;
extern int barrelSpeed;

extern std::vector<std::string> mapLayout;

#endif