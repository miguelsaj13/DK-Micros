#ifndef ENTITIES_H
#define ENTITIES_H

struct Player {
    int x;
    int y;
    int lives;
    int score;
};

struct Barrel {
    int x;
    int y;
    bool active;
};

#endif