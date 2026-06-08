#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>
#include <pthread.h>
#include <atomic>

#include "entities.h"
#include <semaphore.h>

/*
 * Estado global compartido por los módulos de juego.
 * Estas variables son utilizadas por los hilos de entrada, lógica y render.
 */
extern std::atomic<bool> running;   // Controla si la partida sigue activa.
extern std::atomic<bool> gameOver;  // Indica si la partida terminó en derrota.

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
extern std::atomic<int> barrelSpeed; // Velocidad base de movimiento de barriles.

extern std::vector<std::string> mapLayout; // Mapa actual del nivel en uso.

#endif