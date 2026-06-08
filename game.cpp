/*
 * Núcleo de la lógica del juego.
 * Gestiona la ejecución de hilos, el avance de estado y las reglas del escenario.
 */
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
#include <atomic>

using namespace std;

pthread_mutex_t playerMutex;
pthread_mutex_t barrelMutex;
pthread_mutex_t eventMutex;
pthread_mutex_t inputMutex;
pthread_cond_t inputCond;
pthread_mutex_t tickMutex;
pthread_cond_t tickCond;
pthread_barrier_t frameBarrier;
sem_t barrelSemaphore;

Player player = {3, 25, 3, 0, false, 0, 0};
DonkeyKong donkeyKong = {63, 2};

vector<Barrel> barrels;

string gameEvent = "";

std::atomic<bool> running(true);
std::atomic<bool> gameOver(false);

std::atomic<char> lastKey('\0');

std::atomic<bool> hammerActive(false);
std::atomic<bool> hammerCollected(false);

std::atomic<int> hammerTimer(0);
std::atomic<int> levelGame(1);
std::atomic<int> barrelSpeed(1200000);

vector<string> originalMap = {

"================================================================================",
"!                                                                              !",
"!               |           DONKEY KONG                                        !",
"!          =====|==============================================================!",
"!               |                                                              !",
"!               |                                                              !",
"!                                                                              !",
"!==========================================================================    !",
"!                                                                              !",
"!                                                                         =====!",
"!   |                                                                          !",
"!===|========= ================================================================!",
"!   |                                                                          !",
"!   |                                                                          !",
"!                                                                          |   !",
"!=================================================================== ======|===!",
"!                                                                          |   !",
"!                                                                          |   !",
"!                                                                              !",
"!    ============================H=============================================!",
"!                                                                              !",
"!====                                                                          !",
"!                                                                   |          !",
"!===================================================================|==        !",
"!                                                                   |          !",
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
bool isLadder(int y, int x)
{
    if(y < 0 || y >= static_cast<int>(mapLayout.size()))
        return false;

    if(x < 0 || x >= static_cast<int>(mapLayout[y].size()))
        return false;

    return mapLayout[y][x] == '|';
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

    mvprintw(17, 5, "GAME ELEMENTS:");
    mvprintw(18, 5, "M -> Mario");
    mvprintw(19, 5, "K -> Donkey Kong");
    mvprintw(20, 5, "O -> Barrel");
    mvprintw(21, 5, "H -> Hammer");

    mvprintw(24, 20, "Press any key to return...");
    refresh();

    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

/*
 * Lógica del jugador.
 * Procesa entrada, salto, gravedad, recolección de martillo y final de nivel.
 */
void *playerThread(void *arg) {
    pthread_barrier_wait(&frameBarrier);
    while(running.load()) {

        pthread_mutex_lock(&playerMutex);

        switch(lastKey.load()) {

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

                if(isLadder(player.y, player.x) || isLadder(player.y + 1, player.x))
                    player.y++;

                break;

            case ' ':

                if(!player.jumping && !isLadder(player.y, player.x)) {

                    player.jumping = true;
                    player.jumpFrames = 6;
                    player.jumpHeight = 0;
                }

                break;
        }

        if(player.jumping) {

            if(player.jumpFrames > 3) {

                if(!isPlatform(player.y - 1, player.x)) {

                    player.y--;
                    player.jumpHeight++;
                }
            }
            else {

                if(player.jumpHeight > 0) {

                    if(!isSupport(player.y + 1, player.x))
                    {
                        player.y++;
                        player.jumpHeight--;
                    }
                    else
                    {
                        player.jumping = false;
                        player.jumpHeight = 0;
                        player.jumpFrames = 0;
                    }
                }
            }

            player.jumpFrames--;

            if(player.jumpFrames <= 0) {

                player.jumping = false;
            }
        }

        if(!player.jumping &&
           !isSupport(player.y + 1, player.x) &&
           !isLadder(player.y, player.x) && !isLadder(player.y + 1, player.x))
        {
            player.y++;
        }

        if(!hammerCollected.load() &&
           isHammer(player.y, player.x))
        {
            hammerActive = true;
            hammerCollected = true;
            hammerTimer = 800;

            pthread_mutex_lock(&eventMutex);
            gameEvent = "Recogiste el martillo!";
            pthread_mutex_unlock(&eventMutex);

            for(auto &row : mapLayout) {

                for(char &c : row) {

                    if(c == 'H')
                        c = '=';
                }
            }
        }

        if(hammerActive.load()) {

            hammerTimer--;

            if(hammerTimer.load() <= 0)
                hammerActive = false;
        }

        if(player.y <= 2) {

            pthread_mutex_lock(&eventMutex);
            gameEvent = "Meta cumplida!";
            pthread_mutex_unlock(&eventMutex);

            player.score += 500;

            running = false;
            gameOver = false;
            pthread_mutex_lock(&tickMutex);
            pthread_cond_broadcast(&tickCond);
            pthread_mutex_unlock(&tickMutex);
        }

        lastKey.store('\0');

        pthread_mutex_unlock(&playerMutex);
        
        pthread_mutex_lock(&tickMutex);
        pthread_cond_wait(&tickCond, &tickMutex);
        pthread_mutex_unlock(&tickMutex);
    }
    return nullptr;
}
/*
 * Hilo de tiempo global.
 * Genera ticks periódicos para sincronizar los hilos del juego.
 */
void *gameClockThread(void *arg)
{
    while(running.load())
    {
        usleep(50000); // 50 ms por tick

        pthread_mutex_lock(&tickMutex);
        pthread_cond_broadcast(&tickCond);
        pthread_mutex_unlock(&tickMutex);
    }

    return nullptr;
}

/*
 * Generador de barriles.
 * Añade un nuevo barril al vector compartido en intervalos regulares.
 */
void *donkeyKongThread(void *arg)
{
    pthread_barrier_wait(&frameBarrier);
    int spawnCounter = 0;

    while(running.load())
    {
        spawnCounter++;

        if(spawnCounter < 24)
        {
            pthread_mutex_lock(&tickMutex);
            pthread_cond_wait(&tickCond, &tickMutex);
            pthread_mutex_unlock(&tickMutex);

            continue;
        }

        spawnCounter = 0;

        pthread_mutex_lock(&barrelMutex);

        Barrel barrel;

        barrel.x = donkeyKong.x - 1;
        barrel.y = donkeyKong.y + 1;
        barrel.active = true;
        barrel.direction = -1;
        barrel.alreadyJumped = false;

        barrels.push_back(barrel);

        sem_post(&barrelSemaphore);

        pthread_mutex_unlock(&barrelMutex);
        
        pthread_mutex_lock(&tickMutex);
        pthread_cond_wait(&tickCond, &tickMutex);
        pthread_mutex_unlock(&tickMutex);
    }

    return nullptr;
}
/*
 * Lógica del movimiento de barriles.
 * Actualiza posición, detecta colisiones y elimina barriles inactivos.
 */
void *barrelMovement(void *arg) {

    pthread_barrier_wait(&frameBarrier);
    int barrelTickCounter = 0;

    while(running.load()) {

        barrelTickCounter++;

        if(barrelTickCounter < 3)
        {
            pthread_mutex_lock(&tickMutex);
            pthread_cond_wait(&tickCond, &tickMutex);
            pthread_mutex_unlock(&tickMutex);

            continue;
        }

        barrelTickCounter = 0;

        pthread_mutex_lock(&barrelMutex);

        for(auto &barrel : barrels) {

            if(!barrel.active)
                continue;

            pthread_mutex_lock(&playerMutex);

            bool jumpedBarrel =
                !barrel.alreadyJumped &&
                player.jumping &&
                abs(barrel.x - player.x) <= 1 &&
                barrel.y > player.y;

            pthread_mutex_unlock(&playerMutex);

            if(jumpedBarrel)
            {
                barrel.alreadyJumped = true;

                pthread_mutex_lock(&eventMutex);
                gameEvent = "Barril saltado!";
                pthread_mutex_unlock(&eventMutex);

                pthread_mutex_lock(&playerMutex);
                player.score += 50;
                pthread_mutex_unlock(&playerMutex);
            }

            pthread_mutex_lock(&playerMutex);

            bool collision =
                barrel.x == player.x &&
                barrel.y == player.y;

            pthread_mutex_unlock(&playerMutex);

            if(collision) {

                if(hammerActive.load()) {

                    barrel.active = false;

                    pthread_mutex_lock(&playerMutex);
                    player.score += 100;
                    pthread_mutex_unlock(&playerMutex);

                    pthread_mutex_lock(&eventMutex);
                    gameEvent = "Barril destruido!";
                    pthread_mutex_unlock(&eventMutex);
                }
                else {

                    pthread_mutex_lock(&eventMutex);
                    gameEvent = "Perdiste una vida ...";
                    pthread_mutex_unlock(&eventMutex);

                    pthread_mutex_lock(&playerMutex);

                    player.lives--;

                    player.x = 3;
                    player.y = 25;

                    player.jumping = false;
                    player.jumpFrames = 0;
                    player.jumpHeight = 0;

                    lastKey.store('\0');

                    bool dead = (player.lives <= 0);

                    pthread_mutex_unlock(&playerMutex);

                    if(dead) {

                        running = false;
                        gameOver = true;

                        pthread_mutex_lock(&tickMutex);
                        pthread_cond_broadcast(&tickCond);
                        pthread_mutex_unlock(&tickMutex);
                    }
                }

                continue;
            }

            if(!isSupport(barrel.y + 1, barrel.x)) {

                barrel.y++;

                if(barrel.y >= static_cast<int>(mapLayout.size()) - 1) {

                    barrel.active = false;
                    continue;
                }
            }
            else {

                barrel.x += barrel.direction;

                if(barrel.x <= 1)
                    barrel.direction = 1;

                if(barrel.x >= 78)
                    barrel.direction = -1;
            }

            pthread_mutex_lock(&playerMutex);

            collision =
                abs(barrel.x - player.x) <= 1 &&
                abs(barrel.y - player.y) <= 1;

            pthread_mutex_unlock(&playerMutex);

            if(collision) {

                if(hammerActive.load()) {

                    barrel.active = false;

                    pthread_mutex_lock(&playerMutex);
                    player.score += 100;
                    pthread_mutex_unlock(&playerMutex);

                    pthread_mutex_lock(&eventMutex);
                    gameEvent = "Barril destruido!";
                    pthread_mutex_unlock(&eventMutex);
                }
                else {

                    pthread_mutex_lock(&eventMutex);
                    gameEvent = "Perdiste una vida ...";
                    pthread_mutex_unlock(&eventMutex);

                    pthread_mutex_lock(&playerMutex);

                    player.lives--;

                    player.x = 3;
                    player.y = 25;

                    player.jumping = false;
                    player.jumpFrames = 0;
                    player.jumpHeight = 0;

                    lastKey.store('\0');

                    bool dead = (player.lives <= 0);

                    pthread_mutex_unlock(&playerMutex);

                    if(dead) {

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
                    return !b.active;
                }),
            barrels.end()
        );

        pthread_mutex_unlock(&barrelMutex);

        pthread_mutex_lock(&tickMutex);
        pthread_cond_wait(&tickCond, &tickMutex);
        pthread_mutex_unlock(&tickMutex);
    }

    return nullptr;
}
/*
 * Hilo de renderizado.
 * Espera ticks de sincronización y dibuja el estado actual en pantalla.
 */
void *renderThread(void *arg)
{
    pthread_barrier_wait(&frameBarrier);
    while(running.load())
    {
        pthread_mutex_lock(&tickMutex);
        pthread_cond_wait(&tickCond, &tickMutex);
        pthread_mutex_unlock(&tickMutex);

        drawGame();
    }

    return nullptr;
}
/*
 * Inicializa el estado de la partida y ejecuta la cadena de hilos hasta su término.
 * Devuelve la puntuación final para su almacenamiento.
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

    pthread_mutex_init(&playerMutex, nullptr);
    pthread_mutex_init(&barrelMutex, nullptr);
    pthread_mutex_init(&eventMutex, nullptr);
    pthread_mutex_init(&inputMutex, nullptr);
    pthread_cond_init(&inputCond, nullptr);
    pthread_mutex_init(&tickMutex, nullptr);
    pthread_cond_init(&tickCond, nullptr);
    pthread_barrier_init(&frameBarrier, nullptr, 4);
    sem_init(&barrelSemaphore, 0, 0);

    pthread_mutex_lock(&barrelMutex);
    barrels.clear();
    pthread_mutex_unlock(&barrelMutex);

    running = true;
    gameOver = false;

    pthread_t inputT;
    pthread_t playerT;
    pthread_t renderT;
    pthread_t donkeyT;
    pthread_t moveT;
    pthread_t clockT;

    pthread_create(&inputT, nullptr, inputThread, nullptr);
    pthread_create(&clockT, nullptr, gameClockThread, nullptr);
    pthread_create(&playerT, nullptr, playerThread, nullptr);
    pthread_create(&renderT, nullptr, renderThread, nullptr);
    pthread_create(&donkeyT, nullptr, donkeyKongThread, nullptr);
    pthread_create(&moveT, nullptr, barrelMovement, nullptr);

    pthread_join(inputT, nullptr);
    pthread_join(clockT, nullptr);
    pthread_join(playerT, nullptr);
    pthread_join(renderT, nullptr);
    pthread_join(donkeyT, nullptr);
    pthread_join(moveT, nullptr);

    clear();

    if(gameOver.load())
        mvprintw(10, 30, "GAME OVER");
    else
        mvprintw(10, 30, "YOU WIN");

    mvprintw(12, 30, "Final Score: %d", player.score);

    flushinp();

    nodelay(stdscr, FALSE);
    echo();

    char name[50] = "";

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

    pthread_mutex_destroy(&playerMutex);
    pthread_mutex_destroy(&barrelMutex);
    pthread_mutex_destroy(&eventMutex);
    pthread_mutex_destroy(&inputMutex);
    pthread_cond_destroy(&inputCond);
    pthread_mutex_destroy(&tickMutex);
    pthread_cond_destroy(&tickCond);
    sem_destroy(&barrelSemaphore);
    pthread_barrier_destroy(&frameBarrier);

    return finalScore;
}