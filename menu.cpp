#include <ncurses.h>
#include "menu.h"
#include "game.h"

/**
 * Muestra el menú principal, lee la opción del jugador y lanza la acción adecuada.
 * Variables:
 *   int option - opción seleccionada por el jugador.
 */
void menu() {

    int option; // opción seleccionada por el jugador.

    while(true) {

        clear();

        mvprintw(5, 28, "DONKEY KONG ASCII");

        mvprintw(9, 30, "1. Start Game");
        mvprintw(10, 30, "2. Two Players");
        mvprintw(11, 30, "3. Instructions");
        mvprintw(12, 30, "4. Scores");
        mvprintw(13, 30, "5. Exit");

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
            {
                clear();
                mvprintw(10, 25, "PLAYER 1 TURN");
                refresh();

                nodelay(stdscr, FALSE);
                getch();
                nodelay(stdscr, TRUE);

                // ejecutar la partida para el jugador 1 y guardar su puntuación.
                int score1 = startGame();

                clear();

                mvprintw(10, 25, "PLAYER 2 TURN");
                refresh();

                nodelay(stdscr, FALSE);
                getch();
                nodelay(stdscr, TRUE);

                int score2 = startGame();

                clear();

                mvprintw(8, 25, "RESULTS");

                mvprintw(10, 25, "Player 1 Score: %d", score1);

                mvprintw(11, 25, "Player 2 Score: %d", score2);

                if(score1 > score2)
                    mvprintw(13, 25, "PLAYER 1 WINS!");
                
                    else if(score2 > score1)
                        mvprintw(13, 25, "PLAYER 2 WINS!");
                    else
                        mvprintw(13, 25, "TIE!");

                    mvprintw(16, 25, "Press any key...");

                    refresh();

                    nodelay(stdscr, FALSE);
                    getch();
                    nodelay(stdscr, TRUE);

                    break;
                }
            
            case '3':
                instructions();
                break;

            case '4':
                showScores();
                break;

            case '5':
                return;
        }
    }
}