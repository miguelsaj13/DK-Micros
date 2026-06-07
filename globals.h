#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>
#include <pthread.h>
#include <atomic>

#include "entities.h"
#include <semaphore.h>

extern std::atomic<bool> running;
extern std::atomic<bool> gameOver;

extern Player player;
extern DonkeyKong donkeyKong;
extern std::vector<Barrel> barrels;

extern std::string gameEvent;

extern pthread_mutex_t playerMutex;
extern pthread_mutex_t barrelMutex;
extern pthread_mutex_t eventMutex;

extern pthread_mutex_t inputMutex;
extern pthread_cond_t inputCond;
extern pthread_barrier_t frameBarrier;

extern pthread_mutex_t tickMutex;
extern pthread_cond_t tickCond;
extern sem_t barrelSemaphore;

extern std::atomic<char> lastKey;

extern std::atomic<bool> hammerActive;
extern std::atomic<bool> hammerCollected;
extern std::atomic<int> hammerTimer;

extern std::atomic<int> levelGame;
extern std::atomic<int> barrelSpeed;

extern std::vector<std::string> mapLayout;

#endif