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

Player player = {2, 24, 3, 0};

vector<Barrel> barrels;

char lastKey = '\0';

bool hammerActive = false;
int hammerTimer = 0;

int levelGame = 1;
int barrelSpeed = 2000000;

vector<string> mapLayout = {

"================================================================================",
"|                                                                              |",
"|                           DONKEY KONG                             K          |",
"|=======================================================================       |",
"|                            |                                                 |",
"|                            |                                                 |",
"|                            |                                                 |",
"|============================|==========================================       |",
"|                            |                                                 |",
"|                            |                                                 |",
"|                            |                                                 |",
"|=======================================================================       |",
"|                                              |                               |",
"|                                              |                               |",
"|                                              |                               |",
"|==============================================|=========================      |",
"|                |                                                             |",
"|                |                                                             |",
"|                |                                                             |",
"|================|==============================================               |",
"|                                                                              |",
"|                                                                              |",
"|                                                                              |",
"|=======================================================================       |",
"|                                                                              |",
"|                                                                              |",
"================================================================================"

};

bool isLadder(int y, int x) {

    if(y < 0 || y >= mapLayout.size()) return false;
    if(x < 0 || x >= mapLayout[y].size()) return false;

    return mapLayout[y][x] == '|';
}

bool isHammer(int y, int x) {

    if(y < 0 || y >= mapLayout.size()) return false;
    if(x < 0 || x >= mapLayout[y].size()) return false;

    return mapLayout[y][x] == 'H';
}

void saveScore(string name, int score) {

    ofstream file("scores.txt", ios::app);

    if(file.is_open()) {

        file << name << " " << score << endl;
        file.close();
    }
}

void showScores() {

    clear();

    vector<pair<string,int>> scores;

    ifstream file("scores.txt");

    string name;
    int score;

    while(file >> name >> score) {
        scores.push_back({name, score});
    }

    sort(scores.begin(), scores.end(),
        [](auto &a, auto &b) {
            return a.second > b.second;
        });

    mvprintw(2, 30, "TOP SCORES");

    int row = 5;

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

                for(int i = 0; i < 2; i++) {

                    player.y--;
                    drawGame();
                    usleep(70000);
                }

                for(int i = 0; i < 2; i++) {

                    player.y++;
                    drawGame();
                    usleep(70000);
                }

                player.score += 10;

                break;

            case 'p':
            case 'P':

                mvprintw(26, 30, "GAME PAUSED");
                refresh();

                while(true) {

                    int pauseKey = getch();

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

        if(isHammer(player.y, player.x)) {

            hammerActive = true;
            hammerTimer = 300;
        }

        if(hammerActive) {

            hammerTimer--;

            if(hammerTimer <= 0)
                hammerActive = false;
        }

        if(player.y <= 4) {

            levelGame++;

            player.score += 500;

            player.x = 2;
            player.y = 24;

            if(barrelSpeed > 300000)
                barrelSpeed -= 100000;
        }

        lastKey = '\0';

        pthread_mutex_unlock(&gameMutex);

        usleep(70000);
    }

    return NULL;
}

void *barrelSpawner(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        Barrel barrel;

        barrel.x = 70;
        barrel.y = 3;
        barrel.active = true;

        barrels.push_back(barrel);

        pthread_mutex_unlock(&gameMutex);

        usleep(barrelSpeed);
    }

    return NULL;
}

void *barrelMovement(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        for(auto &barrel : barrels) {

            if(barrel.active) {

                barrel.x--;

                if(barrel.x <= 2) {

                    if(barrel.y == 4) {
                        barrel.y = 8;
                        barrel.x = 70;
                    }

                    else if(barrel.y == 8) {
                        barrel.y = 12;
                        barrel.x = 70;
                    }

                    else if(barrel.y == 12) {
                        barrel.y = 16;
                        barrel.x = 70;
                    }

                    else if(barrel.y == 16) {
                        barrel.y = 20;
                        barrel.x = 70;
                    }

                    else {
                        barrel.active = false;
                    }
                }

                if(barrel.x == player.x &&
                   barrel.y == player.y) {

                    if(hammerActive) {

                        barrel.active = false;
                        player.score += 100;

                    } else {

                        player.lives--;

                        player.x = 2;
                        player.y = 24;

                        if(player.lives <= 0) {

                            running = false;
                            gameOver = true;
                        }
                    }
                }
            }
        }

        pthread_mutex_unlock(&gameMutex);

        usleep(150000);
    }

    return NULL;
}

void *renderThread(void *arg) {

    while(running) {

        pthread_mutex_lock(&gameMutex);

        drawGame();

        pthread_mutex_unlock(&gameMutex);

        usleep(33000);
    }

    return NULL;
}

void startGame() {

    player.x = 2;
    player.y = 24;
    player.lives = 3;
    player.score = 0;

    barrels.clear();

    running = true;
    gameOver = false;

    pthread_mutex_init(&gameMutex, NULL);

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

    echo();

    char name[50];

    mvprintw(15, 20, "Enter your name: ");
    getstr(name);

    noecho();

    saveScore(name, player.score);

    mvprintw(18, 20, "Press any key...");
    refresh();

    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);

    pthread_mutex_destroy(&gameMutex);
}