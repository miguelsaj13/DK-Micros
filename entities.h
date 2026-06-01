#ifndef ENTITIES_H
#define ENTITIES_H

struct Player {

    int x;
    int y;
    int lives;
    int score;

    bool jumping;
    int jumpFrames;
    int jumpHeight;
};

struct Barrel {
    int x;
    int y;
    bool active;
    int direction;
};

#endif