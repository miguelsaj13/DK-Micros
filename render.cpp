#include <ncurses.h>
#include "render.h"
#include "globals.h"

void drawGame() {

    clear();

    for(int i = 0; i < mapLayout.size(); i++) {

        mvaddstr(i, 0, mapLayout[i].c_str());
    }

    mvprintw(0, 2, "Lives: %d", player.lives);
    mvprintw(0, 20, "Score: %d", player.score);
    mvprintw(0, 40, "Level: %d", levelGame);

    if(hammerActive) {
        mvprintw(0, 60, "HAMMER");
    }

    mvaddch(player.y, player.x, 'M');

    for(auto &barrel : barrels) {

        if(barrel.active) {

            mvaddch(barrel.y, barrel.x, 'O');
        }
    }

    refresh();
}