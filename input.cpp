#include <unistd.h>
#include <ncurses.h>

#include "globals.h"
#include "input.h"

/**
 * Hilo encargado de capturar la entrada del teclado.
 */
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