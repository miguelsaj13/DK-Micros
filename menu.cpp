#include <ncurses.h>
#include "menu.h"
#include "game.h"

void menu() {

    int option;

    while(true) {

        clear();

        mvprintw(5, 28, "DONKEY KONG ASCII");

        mvprintw(9, 30, "1. Start Game");
        mvprintw(10, 30, "2. Instructions");
        mvprintw(11, 30, "3. Scores");
        mvprintw(12, 30, "4. Exit");

        mvprintw(15, 25, "Select option: ");

        refresh();

        nodelay(stdscr, FALSE);

        option = getch();

        nodelay(stdscr, TRUE);

        switch(option) {

            case '1':
                startGame();
                break;

            case '2':
                instructions();
                break;

            case '3':
                showScores();
                break;

            case '4':
                return;
        }
    }
}