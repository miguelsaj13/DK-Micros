#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <ncurses.h>

#include "game.h"
#include "render.h"
#include "input.h"
#include "globals.h"

using namespace std;

bool running = true;
bool gameOver = false;

pthread_mutex_t gameMutex;

Player player = {3, 25, 3, 0, false, 0, 0};

vector<Barrel> barrels;

char lastKey = '\0';

bool hammerActive = false;
bool hammerCollected = false;
string gameEvent = "";
int hammerTimer = 0;

int levelGame = 1;
int barrelSpeed = 1200000;

vector<string> originalMap = {

"================================================================================",
"!                                                                              !",
"!               |           DONKEY KONG                             K          !",
"!          =====|==============================================================!",
"!               |                                                              !",
"!               |                                                              !",
"!               |                                                    |         !",
"!===============|====================================================|====     !",
"!                                                                    |         !",
"!                                                                    |         !",
"!   |                                                                |         !",
"!===|========= ======================================================|=========!",
"!   |                                                                          !",
"!   |                                                                          !",
"!   |                                                                      |   !",
"!===|=============================================================== ======|===!",
"!                                                                          |   !",
"!                                                                          |   !",
"!                |                                                         |   !",
"!==== ===========|===============H=========================================|===!",
"!                |                                                             !",
"!                |                                                             !",
"!                |                                                  |          !",
"!================|==================================================|==        !",
"!                                                                   |          !",
"!                                                                              !",
"================================================================================"

};

vector<string> mapLayout = originalMap;

/**
 * Comprueba si la celda dada forma parte de una escalera.
 * Variables:
 *   int y - fila en el mapa.
 *   int x - columna en el mapa.
 */
bool isLadder(int y, int x) {

    if(y < 0 || y >= mapLayout.size())
        return false;

    if(x < 1 || x >= mapLayout[y].size() - 1)
        return false;

    return mapLayout[y][x] == '|' ||
           mapLayout[y][x - 1] == '|' ||
           mapLayout[y][x + 1] == '|';
}
/**
 * Comprueba si la celda dada es un soporte sólido o parte de una escalera.
 * Variables:
 *   int y - fila en el mapa.
 *   int x - columna en el mapa.
 */
bool isSupport(int y, int x)
{
    if(y < 0 || y >= mapLayout.size())
        return false;

    if(x < 0 || x >= mapLayout[y].size())
        return false;

    return mapLayout[y][x] == '=' ||
           mapLayout[y][x] == '|';
}

/**
 * Comprueba si hay un martillo en las celdas vecinas a la posición dada.
 * Variables:
 *   int y - fila del jugador.
 *   int x - columna del jugador.
 *   int ny - fila de la celda comprobada.
 *   int nx - columna de la celda comprobada.
 */
bool isHammer(int y, int x) {

    for(int dy = -1; dy <= 1; dy++) {

        for(int dx = -1; dx <= 1; dx++) {

            int ny = y + dy; // fila a comprobar alrededor del jugador.
            int nx = x + dx; // columna a comprobar alrededor del jugador.

            if(ny >= 0 &&
               ny < mapLayout.size() &&
               nx >= 0 &&
               nx < mapLayout[ny].size())
            {
                if(mapLayout[ny][nx] == 'H')
                    return true;
            }
        }
    }

    return false;
}

/**
 * Comprueba si la celda indicada pertenece a una plataforma.
 * Variables:
 *   int y - fila del mapa.
 *   int x - columna del mapa.
 */
bool isPlatform(int y, int x)
{
    if(y < 0 || y >= mapLayout.size())
        return false;

    if(x < 0 || x >= mapLayout[y].size())
        return false;

    return mapLayout[y][x] == '=';
}

/**
 * Añade una entrada de nombre y puntuación al archivo de resultados.
 * Variables:
 *   string name - nombre del jugador.
 *   int score - puntuación final del jugador.
 *   ofstream file - flujo de salida al archivo scores.txt.
 */
void saveScore(string name, int score) {

    ofstream file("scores.txt", ios::app);

    if(file.is_open()) {

        file << name << " " << score << endl;
        file.close();
    }
}

/**
 * Muestra la tabla de puntuaciones en pantalla.
 * Variables:
 *   vector<pair<string,int>> scores - resultados leídos del archivo.
 *   ifstream file - flujo de entrada desde scores.txt.
 *   string name - nombre leído de cada línea.
 *   int score - puntuación leída de cada línea.
 *   int row - fila de impresión en la pantalla.
 */
void showScores() {

    clear();

    // lista de parejas <nombre, puntuación> leídas del archivo.
    vector<pair<string,int>> scores;

    ifstream file("scores.txt");

    string name; // nombre del jugador leído del fichero.
    int score; // puntuación correspondiente a ese nombre.

    while(file >> name >> score) { // leer cada línea de resultado
        scores.push_back({name, score});
    }

    sort(scores.begin(), scores.end(),
        [](auto &a, auto &b) {
            return a.second > b.second;
        });

    mvprintw(2, 30, "TOP SCORES");

    int row = 5; // fila inicial para pintar los resultados.

    for(auto &s : scores) {

        mvprintw(row, 25, "%s - %d",
            s.first.c_str(),
            s.second);

        row++;
    }

    mvprintw(row + 2, 20, "Press any key...");
    refresh();

    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

/**
 * Muestra la pantalla de instrucciones y espera una pulsación de tecla.
 * No utiliza variables locales adicionales.
 */
void instructions() {

    clear();

    mvprintw(2, 30, "INSTRUCTIONS");

    mvprintw(5, 5, "OBJECTIVE:");
    mvprintw(6, 5, "Reach the top while avoiding barrels.");

    mvprintw(9, 5, "CONTROLS:");
    mvprintw(10, 5, "A -> Move Left");
    mvprintw(11, 5, "D -> Move Right");
    mvprintw(12, 5, "W -> Climb");
    mvprintw(13, 5, "S -> Down");
    mvprintw(14, 5, "SPACE -> Jump");
    mvprintw(15, 5, "P -> Pause");
    mvprintw(16, 5, "Q -> Quit");

    mvprintw(19, 5, "GAME ELEMENTS:");
    mvprintw(20, 5, "M -> Mario");
    mvprintw(21, 5, "K -> Donkey Kong");
    mvprintw(22, 5, "O -> Barrel");
    mvprintw(23, 5, "H -> Hammer");

    mvprintw(26, 20, "Press any key to return...");
    refresh();

    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

/**
 * Ejecuta el hilo del jugador, procesando teclas, gravedad y colisiones.
 * Variables:
 *   int pauseKey - tecla usada para reanudar el juego cuando está en pausa.
 */
void *playerThread(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        switch(lastKey) {

            case 'a':
            case 'A':

                if(player.x > 1)
                    player.x--;

                break;

            case 'd':
            case 'D':

                if(player.x < 78)
                    player.x++;

                break;

            case 'w':
            case 'W':

                if(isLadder(player.y, player.x))
                    player.y--;

                break;

            case 's':
            case 'S':

                if(isLadder(player.y, player.x))
                    player.y++;

                break;

            case ' ':

                if(!player.jumping) {

                    player.jumping = true;
                    player.jumpFrames = 10;
                    player.jumpHeight = 0;
                }

                break;

            case 'p':
            case 'P':

                mvprintw(26, 30, "GAME PAUSED");
                refresh();

                while(true) {

                    int pauseKey = getch(); // tecla usada para reanudar el juego.

                    if(pauseKey == 'p' || pauseKey == 'P')
                        break;

                    usleep(50000);
                }

                break;

            case 'q':
            case 'Q':

                running = false;
                break;
        }

        if(player.jumping) {

            if(player.jumpFrames > 5) {

                if(!isPlatform(player.y - 1, player.x)) {

                    player.y--;
                    player.jumpHeight++;
                }
            }
            else {

                if(player.jumpHeight > 0) {

                    player.y++;
                    player.jumpHeight--;
                }
            }

            player.jumpFrames--;

            if(player.jumpFrames <= 0) {

                player.jumping = false;
            }
        }

        if(!player.jumping &&
           !isSupport(player.y + 1, player.x) &&
           !isLadder(player.y, player.x))
        {
            player.y++;
        }


        if(!hammerCollected && isHammer(player.y, player.x)) {

            hammerActive = true;
            hammerCollected = true;
            hammerTimer = 800;
            gameEvent = "Recogiste el martillo!";
            for(auto &row : mapLayout) {

                for(char &c : row) {

                    if(c == 'H')
                        c = '=';
                }
            }
        }
        if(hammerActive) {

            hammerTimer--;

            if(hammerTimer <= 0)
                hammerActive = false;
        }


        if(player.y <= 2) {
            gameEvent = "Meta cumplida!";
            player.score += 500;

            running = false;
            gameOver = false;
        }

        lastKey = '\0';

        pthread_mutex_unlock(&gameMutex);

        usleep(50000);
    }

    return NULL;
}

/**
 * Genera barriles nuevos desde la posición inicial del nivel.
 * Variables:
 *   Barrel barrel - estructura temporal que representa el nuevo barril.
 */
void *barrelSpawner(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        Barrel barrel; // instancia temporal para el barril nuevo.

        barrel.x = 68;
        barrel.y = 1;
        barrel.active = true;
        barrel.direction = -1;

        barrels.push_back(barrel);

        pthread_mutex_unlock(&gameMutex);

        usleep(barrelSpeed);
    }

    return NULL;
}

/**
 * Gestiona el movimiento de los barriles y su interacción con el jugador.
 * Variables:
 *   Barrel &barrel - referencia al barril que se procesa.
 */
void *barrelMovement(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        // procesar cada barril activo en la lista.
        for(auto &barrel : barrels) {

            if(!barrel.active)
                continue;

            if(abs(barrel.x - player.x) <= 1 &&
               abs(barrel.y - player.y) <= 1) {

                if(hammerActive) {

                    barrel.active = false;
                    player.score += 100;
                    gameEvent = "Barril destruido!";
                }
                else {

                    gameEvent = "Perdiste una vida ...";
                    player.lives--;

                    player.x = 3;
                    player.y = 25;

                    player.jumping = false;
                    player.jumpFrames = 0;
                    player.jumpHeight = 0;

                    lastKey = '\0';

                    if(player.lives <= 0) {

                        running = false;
                        gameOver = true;
                    }
                }

                continue;
            }

            if(!isSupport(barrel.y + 1, barrel.x)) {

                barrel.y++;

                if(barrel.y >= mapLayout.size() - 1) {

                    barrel.active = false;
                    continue;
                }
            }
            else {

                barrel.x += barrel.direction;

                if(barrel.x <= 1) {

                    barrel.direction = 1;
                }

                if(barrel.x >= 78) {

                    barrel.direction = -1;
                }
            }

            if(abs(barrel.x - player.x) <= 1 &&
               abs(barrel.y - player.y) <= 1) {

                if(hammerActive) {

                    barrel.active = false;
                    gameEvent = "Barril saltado!";
                    player.score += 100;
                }
                else {
                    gameEvent = "Perdiste una vida ...";
                    player.lives--;

                    player.x = 3;
                    player.y = 25;

                    player.jumping = false;
                    player.jumpFrames = 0;
                    player.jumpHeight = 0;

                    lastKey = '\0';

                    if(player.lives <= 0) {

                        running = false;
                        gameOver = true;
                    }
                }
            }
        }

        barrels.erase(
            remove_if(
                barrels.begin(),
                barrels.end(),
                [](const Barrel& b) {
                    // eliminar barriles marcados como inactivos.
                    return !b.active;
                }),
            barrels.end()
        );

        pthread_mutex_unlock(&gameMutex);

        usleep(150000);
    }

    return NULL;
}

/**
 * Ejecuta el hilo de renderizado para refrescar la pantalla periódicamente.
 * No utiliza variables locales adicionales.
 */
void *renderThread(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        drawGame();

        pthread_mutex_unlock(&gameMutex);

        usleep(33000);
    }

    return NULL;
}

/**
 * Inicializa el estado completo del juego y ejecuta los hilos de entrada, lógica y renderizado.
 * Variables:
 *   pthread_t inputT - hilo de entrada de teclado.
 *   pthread_t playerT - hilo de lógica del jugador.
 *   pthread_t renderT - hilo de renderizado.
 *   pthread_t spawnT - hilo de generación de barriles.
 *   pthread_t moveT - hilo de movimiento de barriles.
 *   char name[50] - buffer para el nombre ingresado por el jugador.
 */
int startGame() {
    player.x = 3;
    player.y = 25;
    player.lives = 3;
    player.score = 0;
    player.jumping = false;
    player.jumpFrames = 0;
    player.jumpHeight = 0;
    hammerActive = false;
    hammerCollected = false;
    hammerTimer = 0;
    levelGame = 1;
    barrelSpeed = 1200000;
    lastKey = '\0';

    mapLayout = originalMap;

    barrels.clear();

    running = true;
    gameOver = false;

    pthread_mutex_init(&gameMutex, NULL);

    // identificadores de hilos para entrada, jugador, render, spawn y movimiento.
    pthread_t inputT;
    pthread_t playerT;
    pthread_t renderT;
    pthread_t spawnT;
    pthread_t moveT;

    pthread_create(&inputT, NULL, inputThread, NULL);
    pthread_create(&playerT, NULL, playerThread, NULL);
    pthread_create(&renderT, NULL, renderThread, NULL);
    pthread_create(&spawnT, NULL, barrelSpawner, NULL);
    pthread_create(&moveT, NULL, barrelMovement, NULL);

    pthread_join(inputT, NULL);
    pthread_join(playerT, NULL);
    pthread_join(renderT, NULL);
    pthread_join(spawnT, NULL);
    pthread_join(moveT, NULL);

    clear();

    if(gameOver)
        mvprintw(10, 30, "GAME OVER");
    else
        mvprintw(10, 30, "YOU WIN");

    mvprintw(12, 30, "Final Score: %d", player.score);

    flushinp();

    nodelay(stdscr, FALSE);
    echo();

    char name[50] = ""; // buffer para el nombre del jugador.

    mvprintw(15, 20, "Enter your name: ");
    refresh();

    wgetnstr(stdscr, name, 49);

    saveScore(string(name), player.score);

    noecho();
    nodelay(stdscr, TRUE);

    mvprintw(18, 20, "Press any key...");
    refresh();

    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);

    int finalScore = player.score;

    pthread_mutex_destroy(&gameMutex);

    return finalScore;
}