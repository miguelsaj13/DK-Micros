#include <ncurses.h>
#include <pthread.h>
#include "menu.h"

/**
 * Inicializa ncurses y ejecuta el menú principal.
 * No declara variables locales adicionales.
 */
int main() {

    initscr();
    noecho();
    cbreak();
    curs_set(0);

    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    start_color();

    menu();

    endwin();

    return 0;
}