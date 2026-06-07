#ifndef ENTITIES_H
#define ENTITIES_H

// Estado dinámico del jugador en el juego.
struct Player {
    int x;
    int y;
    int lives;
    int score;

    bool jumping;
    int jumpFrames;
    int jumpHeight;
};

// Estado dinámico de un barril en movimiento.
struct Barrel {
    int x;
    int y;
    bool active;
    int direction;

    bool alreadyJumped;
};

// Estado de Donkey Kong.
struct DonkeyKong {
    int x;
    int y;
};

#endif