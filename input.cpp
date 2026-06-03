#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include "globals.h"
#include "input.h"

/**
 * Bucle del hilo de entrada que lee teclas y actualiza la variable global lastKey.
 * Variables:
 *   int ch - código de tecla leído desde ncurses.
 */
void *inputThread(void *arg) {

    while(running) {

        int ch = getch(); // código de tecla leído desde ncurses.

        pthread_mutex_lock(&gameMutex);

        if(ch != ERR) {
            lastKey = ch;
        }

        pthread_mutex_unlock(&gameMutex);

        usleep(30000);
    }

    return NULL;
}