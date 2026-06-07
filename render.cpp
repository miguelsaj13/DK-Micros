#include <ncurses.h>
#include <vector>
#include <string>

#include "render.h"
#include "globals.h"

/**
 * Renderiza el estado actual del juego en pantalla.
 */
void drawGame()
{
    Player playerCopy;
    std::vector<Barrel> barrelsCopy;
    std::string eventCopy;

    // Copiar jugador de forma segura
    pthread_mutex_lock(&playerMutex);
    playerCopy = player;
    pthread_mutex_unlock(&playerMutex);

    // Copiar barriles de forma segura
    pthread_mutex_lock(&barrelMutex);
    barrelsCopy = barrels;
    pthread_mutex_unlock(&barrelMutex);

    // Copiar mensaje de evento de forma segura
    pthread_mutex_lock(&eventMutex);
    eventCopy = gameEvent;
    pthread_mutex_unlock(&eventMutex);

    clear();

    // Dibujar mapa
    for(int i = 0; i < static_cast<int>(mapLayout.size()); i++)
    {
        mvaddstr(i, 0, mapLayout[i].c_str());
    }

    // HUD
    mvprintw(0, 2, "Lives: %d", playerCopy.lives);
    mvprintw(0, 20, "Score: %d", playerCopy.score);
    mvprintw(0, 40, "Level: %d", levelGame.load());

    if(hammerActive.load())
    {
        mvprintw(0, 60, "HAMMER");
    }

    mvprintw(29, 2, "Event: %-40s", eventCopy.c_str());

    // Dibujar jugador
    if(playerCopy.y >= 0 &&
       playerCopy.y < static_cast<int>(mapLayout.size()) &&
       playerCopy.x >= 0 &&
       playerCopy.x < static_cast<int>(mapLayout[playerCopy.y].size()))
    {
        mvaddch(playerCopy.y, playerCopy.x, 'M');
    }
    // Dibujar Donkey Kong
    if(donkeyKong.y >= 0 &&
        donkeyKong.y < static_cast<int>(mapLayout.size()) &&
        donkeyKong.x >= 0 &&
        donkeyKong.x < static_cast<int>(mapLayout[donkeyKong.y].size()))
    {
        mvaddch(donkeyKong.y, donkeyKong.x, 'K');
    }
    // Dibujar barriles
    for(const auto &barrel : barrelsCopy)
    {
        if(barrel.active &&
           barrel.y >= 0 &&
           barrel.y < static_cast<int>(mapLayout.size()) &&
           barrel.x >= 0 &&
           barrel.x < static_cast<int>(mapLayout[barrel.y].size()))
        {
            mvaddch(barrel.y, barrel.x, 'O');
        }
    }

    refresh();
}