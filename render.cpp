#include <ncurses.h>
#include "render.h"
#include "globals.h"

void drawGame() {

    clear();

    // Dibujar mapa
    for(int i = 0; i < mapLayout.size(); i++) {

        mvaddstr(i, 0, mapLayout[i].c_str());
    }

    mvprintw(0, 2, "Lives: %d", player.lives);
    mvprintw(0, 20, "Score: %d", player.score);
    mvprintw(0, 40, "Level: %d", levelGame);

    if(hammerActive) {
        mvprintw(0, 60, "HAMMER");
    }
    mvprintw(27, 2, "Event: %-40s", gameEvent.c_str());

    // Dibujar jugador
    if(player.y >= 0 &&
       player.y < mapLayout.size() &&
       player.x >= 0 &&
       player.x < mapLayout[player.y].size())
    {
        mvaddch(player.y, player.x, 'M');
    }

    // Dibujar barriles
    for(auto &barrel : barrels) {

        if(barrel.active &&
           barrel.y >= 0 &&
           barrel.y < mapLayout.size() &&
           barrel.x >= 0 &&
           barrel.x < mapLayout[barrel.y].size())
        {
            mvaddch(barrel.y, barrel.x, 'O');
        }
    }

    refresh();
}