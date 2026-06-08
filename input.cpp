/*
 * Módulo de entrada del juego.
 * Lee el teclado y notifica cambios de estado a través de variables sincronizadas.
 */
#include <unistd.h>
#include <ncurses.h>

#include "globals.h"
#include "input.h"

void *inputThread(void *arg)
{
    while (running.load())
    {
        int ch = getch();

        if (ch != ERR)
        {
            lastKey.store(static_cast<char>(ch));
            pthread_mutex_lock(&inputMutex);
            pthread_cond_signal(&inputCond);
            pthread_mutex_unlock(&inputMutex);
        }

        usleep(30000);
    }

    return nullptr;
}